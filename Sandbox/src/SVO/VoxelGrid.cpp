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

		int celdasPorEje = 1; // Tamaño de cada Cell3D en vóxeles

		// Calculamos el tamaño total del mundo en vóxeles.
		// Debe redondearse al siguiente múltiplo de potencia de 2 para el SVO (ej. 64, 128)
		uint32_t tamañoX = laberinto->GetColumns() * celdasPorEje;
		uint32_t tamañoY = laberinto->GetLevels() * celdasPorEje;
		uint32_t tamañoZ = laberinto->GetRows() * celdasPorEje;

		// Asumiremos un VoxelGrid de tamaño 64 para este ejemplo
		VoxelGrid vGrid(gridSize);

		// 1. Llenar TODO el mundo de piedra (Material = 1)
		for (uint32_t x = 0; x < gridSize; x++) {
			for (uint32_t y = 0; y < gridSize; y++) {
				for (uint32_t z = 0; z < gridSize; z++) {
					uint32_t material = rand();
					vGrid.SetVoxel(x, y, z, material); // 1 = Piedra sólida
				}
			}
		}

		// 2. Esculpir los pasillos iterando sobre el Grid3D
		laberinto->EachCell([&](Maze::Cell3D* cell) {

			// Coordenada base del bloque 3x3x3 en el mundo de vóxeles
			uint32_t bx = cell->GetColumn() * celdasPorEje;
			uint32_t by = cell->GetLevel() * celdasPorEje;
			uint32_t bz = cell->GetRow() * celdasPorEje;

			// Esculpir el centro (aire = 0)
			vGrid.SetVoxel(bx + 1, by + 1, bz + 1, 0);

			// Esculpir pasillos si hay enlaces
			if (cell->IsLinked(cell->GetEast())) {
				vGrid.SetVoxel(bx + 2, by + 1, bz + 1, 0);
			}
			if (cell->IsLinked(cell->GetWest())) {
				vGrid.SetVoxel(bx + 0, by + 1, bz + 1, 0);
			}
			if (cell->IsLinked(cell->GetSouth())) { // Sur suele ser +Z
				vGrid.SetVoxel(bx + 1, by + 1, bz + 2, 0);
			}
			if (cell->IsLinked(cell->GetNorth())) {
				vGrid.SetVoxel(bx + 1, by + 1, bz + 0, 0);
			}
			if (cell->IsLinked(cell->GetUp())) {
				vGrid.SetVoxel(bx + 1, by + 2, bz + 1, 0);
			}
			if (cell->IsLinked(cell->GetDown())) {
				vGrid.SetVoxel(bx + 1, by + 0, bz + 1, 0);
			}
			});

		return vGrid;
	}

}
