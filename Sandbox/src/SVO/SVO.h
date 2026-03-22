#pragma once

#include "VoxelGrid.h"

namespace SVO {

	// El nodo empaquetado de 8 bytes (64 bits) exactos para la GPU
	struct SVONode {
		uint32_t descriptor; // Bits 0-7: Valid, Bits 8-15: Leaf, Bits 16-31: Child Index
		uint32_t material;   // Tu información de color (RGBA o RGB + Propiedad)
	};

	struct OctantResult {
		bool isAir = false;
		bool isSolidLeaf = false;
		uint32_t leafMaterial = 0;
	};

	class SVOBuilder {
	private:
		std::vector<SVONode> m_Nodes;

	public:
		std::vector<SVONode> Build(const VoxelGrid& grid);

	private:
		OctantResult BuildRecursive(uint32_t nodeIndex, uint32_t x, uint32_t y, uint32_t z, uint32_t size, const VoxelGrid& grid);
	};
}
