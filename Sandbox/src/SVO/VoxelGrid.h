#pragma once

#include <Lore.h>

#include "Maze/Base/Grid3D.h"

namespace SVO {

	// Un grid volumétrico temporal para la CPU
	class VoxelGrid {
	private:
		std::vector<uint32_t> m_Data;
		uint32_t m_Size; // Debe ser potencia de 2 (ej. 64, 128)

	public:
		// Cuántos vóxeles de índice ocupa cada celda del laberinto por eje. Junto con
		// el uniform u_VoxelScale del renderer (mundo = índice * voxelScale) define el
		// tamaño físico real de una celda; subir este número sin tocar voxelScale sólo
		// añade resolución (vóxeles más finos) sin cambiar el tamaño del laberinto.
		static constexpr uint32_t kCellsPerAxis = 12;


		VoxelGrid(uint32_t size) : m_Size(size) {
			// Inicializar todo como aire (0)
			m_Data.resize(size * size * size, 0);
		}

		inline void Clear() {
			m_Data.clear();
			m_Size = 0;
		}

		inline void SetVoxel(uint32_t x, uint32_t y, uint32_t z, uint32_t material) {
			LR_ASSERT(x < m_Size && y < m_Size && z < m_Size, "VoxelGrid::SetVoxel out of bounds");
			m_Data[x + y * m_Size + z * m_Size * m_Size] = material;
		}

		inline uint32_t GetVoxel(uint32_t x, uint32_t y, uint32_t z) const {
			LR_ASSERT(x < m_Size && y < m_Size && z < m_Size, "VoxelGrid::GetVoxel out of bounds");
			return m_Data[x + y * m_Size + z * m_Size * m_Size];
		}

		inline uint32_t GetSize() const { return m_Size; }

		std::string ToString() const;

		VoxelGrid CastToVoxels(Maze::Grid3D* laberinto, int gridSize);
	};
}
