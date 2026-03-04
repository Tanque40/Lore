#pragma once

#include "Maze/Base/Grid.h"
#include "Maze/Base/Grid3D.h"

namespace Maze {

	class BinaryTree {
	public:
		static Grid On(Grid grid);
		static void On(Grid3D* grid);
	};

}
