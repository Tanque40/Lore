#pragma once

#include "Maze/Base/Grid.h"
#include "Maze/Base/Grid3D.h"

namespace Maze {

	class Sidewinder {
	public:
		static Grid On(Grid grid);
		static void On(Grid3D* grid);
	};

} // namespace Maze
