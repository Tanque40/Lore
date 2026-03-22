#pragma once

#include "sndbxpch.h"

namespace SVO {

	// Un grid volumétrico temporal para la CPU
	class VoxelGrid {
	private:
		std::vector<uint32_t> m_Data;
		uint32_t m_Size; // Debe ser potencia de 2 (ej. 64, 128)

	public:
		VoxelGrid(uint32_t size) : m_Size(size) {
			// Inicializar todo como aire (0)
			m_Data.resize(size * size * size, 0);
		}

		inline void SetVoxel(uint32_t x, uint32_t y, uint32_t z, uint32_t material) {
			m_Data[x + y * m_Size + z * m_Size * m_Size] = material;
		}

		inline uint32_t GetVoxel(uint32_t x, uint32_t y, uint32_t z) const {
			return m_Data[x + y * m_Size + z * m_Size * m_Size];
		}

		inline uint32_t GetSize() const { return m_Size; }

		inline std::string ToString() const {
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
	};
}
