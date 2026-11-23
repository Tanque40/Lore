#pragma once

#include "Maze/Base/Grid.h"
#include "Maze/Base/Grid3D.h"

namespace Maze {

	class BinaryTree {
	public:
		static Grid On(Grid grid, Random& rng = Random::Global());
		static void On(Grid3D* grid, Random& rng = Random::Global());
	};

}
