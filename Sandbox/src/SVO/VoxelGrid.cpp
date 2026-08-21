#include "sndbxpch.h"

#include "SVO/VoxelGrid.h"

namespace SVO {

	std::string VoxelGrid::ToString() const {
		std::string result;
		uint32_t blocksCount = 0;

		for (uint32_t z = 0; z < m_Size; z++) {
			//result += "Layer " + std::to_string(z) + ":\n";
			for (uint32_t y = 0; y < m_Size; y++) {
				for (uint32_t x = 0; x < m_Size; x++) {
					if (GetVoxel(x, y, z)) {
						//			result += "#";
						blocksCount++;
					}
					else {
						//			result += ".";
					}
				}
				//	result += "\n";
			}
			//result += "\n";
		}

		result += "Total voxel grid size: " + std::to_string(m_Size * m_Size * m_Size) + "\n";
		result += "Total non air blocks: " + std::to_string(blocksCount) + "\n";
		result += "Memory usage: " + std::to_string(blocksCount * sizeof(uint32_t)) + " bytes\n";

		return result;
	}

	VoxelGrid VoxelGrid::CastToVoxels(Maze::Grid3D* laberinto, int gridSize) {
		uint32_t newCellsPerAxis = kCellsPerAxis;

		// El laberinto debe caber dentro del grid: cada celda ocupa un bloque de
		// newCellsPerAxis^3, así que el grid necesita al menos esa extensión.
		uint32_t requiredExtent = std::max({ laberinto->GetColumns(), laberinto->GetLevels(), laberinto->GetRows() }) * newCellsPerAxis;
		gridSize = std::max(gridSize, static_cast<int>(requiredExtent));

		uint32_t i = 0;
		while (static_cast<uint32_t>(std::pow(2, i)) < static_cast<uint32_t>(gridSize)) {
			i++;
		}

		gridSize = static_cast<uint32_t>(std::pow(2, i));

		VoxelGrid vGrid(gridSize);

		// Material constante de "piedra sólida" (RGBA empaquetado, alpha != 0 para que el
		// ray marcher lo trate como opaco). Debe ser constante para que el SVOBuilder pueda
		// fusionar bloques sólidos en vez de tener que subdividir hasta el voxel individual.
		constexpr uint32_t kSolidStoneMaterial = 0xFF808080;

		// 1. Llenar TODO el mundo de piedra sólida
		for (uint32_t x = 0; x < gridSize; x++) {
			for (uint32_t y = 0; y < gridSize; y++) {
				for (uint32_t z = 0; z < gridSize; z++) {
					vGrid.SetVoxel(x, y, z, kSolidStoneMaterial);
				}
			}
		}

		// Ancho del pasillo/cámara en vóxeles de índice (misma proporción que la versión
		// anterior de 1 vóxel de ancho en un bloque de 3, pero escalado a la resolución
		// actual) y desplazamiento para centrarlo dentro del bloque de la celda.
		uint32_t corridorWidth = newCellsPerAxis / 3;
		uint32_t chamberOffset = (newCellsPerAxis - corridorWidth) / 2;

		auto carveBox = [&](uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, uint32_t z0, uint32_t z1) {
			for (uint32_t x = x0; x < x1; x++)
				for (uint32_t y = y0; y < y1; y++)
					for (uint32_t z = z0; z < z1; z++)
						vGrid.SetVoxel(x, y, z, 0);
			};

		// 2. Esculpir los pasillos iterando sobre el Grid3D
		laberinto->EachCell([&](Maze::Cell3D* cell) {

			// Coordenada base del bloque de la celda en el mundo de vóxeles
			uint32_t bx = cell->GetColumn() * newCellsPerAxis;
			uint32_t by = cell->GetLevel() * newCellsPerAxis;
			uint32_t bz = cell->GetRow() * newCellsPerAxis;

			// Cámara central de la celda
			carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
				by + chamberOffset, by + chamberOffset + corridorWidth,
				bz + chamberOffset, bz + chamberOffset + corridorWidth);

			// Pasillos hacia los vecinos enlazados
			if (cell->IsLinked(cell->GetEast())) {
				carveBox(bx + chamberOffset + corridorWidth, bx + newCellsPerAxis,
					by + chamberOffset, by + chamberOffset + corridorWidth,
					bz + chamberOffset, bz + chamberOffset + corridorWidth);
			}
			if (cell->IsLinked(cell->GetWest())) {
				carveBox(bx, bx + chamberOffset,
					by + chamberOffset, by + chamberOffset + corridorWidth,
					bz + chamberOffset, bz + chamberOffset + corridorWidth);
			}
			if (cell->IsLinked(cell->GetSouth())) { // Sur suele ser +Z
				carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
					by + chamberOffset, by + chamberOffset + corridorWidth,
					bz + chamberOffset + corridorWidth, bz + newCellsPerAxis);
			}
			if (cell->IsLinked(cell->GetNorth())) {
				carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
					by + chamberOffset, by + chamberOffset + corridorWidth,
					bz, bz + chamberOffset);
			}
			if (cell->IsLinked(cell->GetUp())) {
				carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
					by + chamberOffset + corridorWidth, by + newCellsPerAxis,
					bz + chamberOffset, bz + chamberOffset + corridorWidth);
			}
			if (cell->IsLinked(cell->GetDown())) {
				carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
					by, by + chamberOffset,
					bz + chamberOffset, bz + chamberOffset + corridorWidth);
			}

			});

		return vGrid;
	}

}
