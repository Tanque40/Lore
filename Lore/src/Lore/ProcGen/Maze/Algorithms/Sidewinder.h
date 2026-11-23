#pragma once

#include "Lore/ProcGen/Maze/Base/Grid.h"
#include "Lore/ProcGen/Maze/Base/Grid3D.h"

namespace Maze {

	class Sidewinder {
	public:
		static Grid On(Grid grid, Random& rng = Random::Global());
		static void On(Grid3D* grid, Random& rng = Random::Global());
	};

} // namespace Maze
