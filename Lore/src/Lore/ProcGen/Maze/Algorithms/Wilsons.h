#pragma once

#include "Lore/ProcGen/Maze/Base/Grid.h"
#include "Lore/ProcGen/Maze/Base/Grid3D.h"

namespace Maze {

	class Wilsons {
	public:
		static Grid On(Grid grid, Random& rng = Random::Global());
		static void On(Grid3D* grid, Random& rng = Random::Global());
	};

}
