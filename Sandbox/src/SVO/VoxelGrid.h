#pragma once

#include "sndbxpch.h"

namespace SVO {

	// El nodo empaquetado de 8 bytes (64 bits) exactos para la GPU
	struct SVONode {
		uint32_t descriptor; // Bits 0-7: Valid, Bits 8-15: Leaf, Bits 16-31: Child Index
		uint32_t material;   // Tu información de color (RGBA o RGB + Propiedad)
	};

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
	};
}
