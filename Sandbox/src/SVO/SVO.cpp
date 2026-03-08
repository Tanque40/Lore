#include "sndbxpch.h"

#include "SVO/SVO.h"

namespace SVO {

	std::vector<SVONode> SVOBuilder::Build(const VoxelGrid& grid) {
		m_Nodes.clear();

		// Reservar memoria inicial para evitar muchas reasignaciones
		m_Nodes.reserve(10000);

		// Insertamos la raíz vacía en el índice 0
		m_Nodes.push_back({ 0, 0 });

		// Iniciamos la recursividad desde el índice 0, esquina (0,0,0) con el tamaño total
		OctantResult rootResult = BuildRecursive(0, 0, 0, 0, grid.GetSize(), grid);

		return m_Nodes;
	}

	OctantResult SVOBuilder::BuildRecursive(uint32_t nodeIndex, uint32_t x, uint32_t y, uint32_t z, uint32_t size, const VoxelGrid& grid) {

		// ---------------------------------------------------------
		// PASO 1: Comprobar el estado de este bloque
		// ---------------------------------------------------------
		bool isAllAir = true;
		bool isAllSolid = true;
		uint32_t firstVoxel = grid.GetVoxel(x, y, z);

		for (uint32_t dz = 0; dz < size; ++dz) {
			for (uint32_t dy = 0; dy < size; ++dy) {
				for (uint32_t dx = 0; dx < size; ++dx) {
					uint32_t currentVoxel = grid.GetVoxel(x + dx, y + dy, z + dz);
					if (currentVoxel != 0) isAllAir = false;
					if (currentVoxel != firstVoxel) isAllSolid = false;
				}
			}
		}

		// ---------------------------------------------------------
		// PASO 2: Casos Base (Poda del árbol)
		// ---------------------------------------------------------
		if (isAllAir) {
			return { true, false, 0 }; // Es aire puro
		}

		if (isAllSolid && firstVoxel != 0) {
			// Es un bloque sólido. Guardamos el material en el nodo.
			m_Nodes[nodeIndex].material = firstVoxel;
			return { false, true, firstVoxel };
		}

		// ---------------------------------------------------------
		// PASO 3: Subdivisión (Es un nodo mezcla / Rama)
		// ---------------------------------------------------------
		uint32_t half = size / 2;

		// Guardamos el índice donde empezarán los 8 hijos
		uint32_t baseChildIndex = static_cast<uint32_t>(m_Nodes.size());

		// Expandimos el vector creando 8 nodos vacíos de golpe
		for (int i = 0; i < 8; ++i) {
			m_Nodes.push_back({ 0, 0 });
		}

		uint8_t validMask = 0;
		uint8_t leafMask = 0;

		// Recorremos los 8 octantes
		for (int i = 0; i < 8; ++i) {
			// Cálculo de coordenadas usando operadores bit a bit (Morton Order)
			uint32_t ox = (i & 1) ? half : 0;
			uint32_t oy = (i & 2) ? half : 0;
			uint32_t oz = (i & 4) ? half : 0;

			uint32_t childIndex = baseChildIndex + i;

			// Llamada recursiva al hijo
			OctantResult childResult = BuildRecursive(childIndex, x + ox, y + oy, z + oz, half, grid);

			// Si el hijo no es aire, encendemos su bit en la máscara Valid
			if (!childResult.isAir) {
				validMask |= (1 << i);
			}

			// Si el hijo resultó ser un bloque sólido perfecto, encendemos su bit en la máscara Leaf
			if (childResult.isSolidLeaf) {
				leafMask |= (1 << i);
			}
		}

		// ---------------------------------------------------------
		// PASO 4: Empaquetar el Descriptor del Padre
		// ---------------------------------------------------------
		uint32_t descriptor = 0;
		descriptor |= (validMask & 0xFF);                   // Bits 0-7
		descriptor |= ((leafMask & 0xFF) << 8);             // Bits 8-15
		descriptor |= ((baseChildIndex & 0xFFFF) << 16);    // Bits 16-31

		m_Nodes[nodeIndex].descriptor = descriptor;

		// Devolvemos que este nodo no es ni aire puro ni una hoja sólida (es una rama)
		return { false, false, 0 };
	}

}
