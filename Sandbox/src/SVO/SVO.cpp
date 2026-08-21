#include "sndbxpch.h"

#include "SVO/SVO.h"

namespace {

	// Mismo layout de bits que DecodificarColor en los shaders: r=bits0-7, g=bits8-15,
	// b=bits16-23, a=bits24-31.
	inline void UnpackRGB(uint32_t material, uint32_t& r, uint32_t& g, uint32_t& b) {
		r = material & 0xFF;
		g = (material >> 8) & 0xFF;
		b = (material >> 16) & 0xFF;
	}

	inline uint32_t PackRGB(uint32_t r, uint32_t g, uint32_t b) {
		// Alpha siempre opaco: sólo se llama con contenido sólido de por medio.
		return 0xFF000000u | (b << 16) | (g << 8) | r;
	}

}

namespace SVO {

	std::vector<SVONode> SVOBuilder::Build(const VoxelGrid& grid) {
		m_Nodes.clear();

		// Reservar memoria inicial para evitar muchas reasignaciones
		m_Nodes.reserve(10000);

		// Insertamos la raíz vacía en el índice 0
		m_Nodes.push_back({ 0, 0, 0 });

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

		for (uint32_t dz = 0; dz < size; dz++) {
			for (uint32_t dy = 0; dy < size; dy++) {
				for (uint32_t dx = 0; dx < size; dx++) {
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
			return { true, false, 0, 0, 0 }; // Es aire puro
		}

		if (isAllSolid && firstVoxel != 0) {
			// Es un bloque sólido. Guardamos el material en el nodo.
			m_Nodes[nodeIndex].material = firstVoxel;
			uint64_t voxelCount = static_cast<uint64_t>(size) * size * size;
			return { false, true, firstVoxel, firstVoxel, voxelCount };
		}

		// ---------------------------------------------------------
		// PASO 3: Subdivisión (Es un nodo mezcla / Rama)
		// ---------------------------------------------------------
		uint32_t half = size / 2;

		// Guardamos el índice donde empezarán los 8 hijos
		uint32_t baseChildIndex = static_cast<uint32_t>(m_Nodes.size());

		// Expandimos el vector creando 8 nodos vacíos de golpe
		for (int i = 0; i < 8; ++i) {
			m_Nodes.push_back({ 0, 0, 0 });
		}

		uint8_t validMask = 0;
		uint8_t leafMask = 0;

		// Acumuladores para el color promedio ponderado del subárbol completo (no sólo de
		// los hijos directos): le permite al ray marcher de la GPU renderizar este nodo
		// como un único bloque "impostor" razonable cuando el LOD por distancia decide no
		// bajar hasta el detalle exacto.
		uint64_t totalSolidCount = 0;
		uint64_t rSum = 0, gSum = 0, bSum = 0;

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

			if (childResult.solidCount > 0) {
				uint32_t r, g, b;
				UnpackRGB(childResult.avgMaterial, r, g, b);
				rSum += static_cast<uint64_t>(r) * childResult.solidCount;
				gSum += static_cast<uint64_t>(g) * childResult.solidCount;
				bSum += static_cast<uint64_t>(b) * childResult.solidCount;
				totalSolidCount += childResult.solidCount;
			}
		}

		// ---------------------------------------------------------
		// PASO 4: Empaquetar el Descriptor del Padre
		// ---------------------------------------------------------
		uint32_t descriptor = 0;
		descriptor |= (validMask & 0xFF);       // Bits 0-7
		descriptor |= ((leafMask & 0xFF) << 8); // Bits 8-15

		uint32_t avgMaterial = 0;
		if (totalSolidCount > 0) {
			avgMaterial = PackRGB(
				static_cast<uint32_t>(rSum / totalSolidCount),
				static_cast<uint32_t>(gSum / totalSolidCount),
				static_cast<uint32_t>(bSum / totalSolidCount));

			uint64_t totalVoxels = static_cast<uint64_t>(size) * size * size;
			uint32_t coverage = static_cast<uint32_t>((totalSolidCount * 255ull) / totalVoxels);
			descriptor |= ((coverage & 0xFF) << 16); // Bits 16-23
		}

		m_Nodes[nodeIndex].descriptor = descriptor;
		m_Nodes[nodeIndex].childIndex = baseChildIndex; // Rango completo de 32 bits, sin truncar
		m_Nodes[nodeIndex].material = avgMaterial;       // Color promedio del subárbol, para el LOD

		// Devolvemos que este nodo no es ni aire puro ni una hoja sólida (es una rama)
		return { false, false, 0, avgMaterial, totalSolidCount };
	}

}
