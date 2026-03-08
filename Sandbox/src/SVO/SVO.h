#pragma once

#include "VoxelGrid.h"

namespace SVO {

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
