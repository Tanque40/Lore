#include "Maze/Algorithms/BinaryTree.h"

#include "Maze/Base/Cell.h"

namespace Maze {

	Grid BinaryTree::On(Grid grid) {
		grid.EachCell([](Cell* cell) {
			std::vector<Cell*> neighbors;
			if (cell->GetNorth()) {
				neighbors.push_back(cell->GetNorth());
			}
			if (cell->GetEast()) {
				neighbors.push_back(cell->GetEast());
			}

			if (!neighbors.empty()) {
				Cell* index = neighbors[rand() % neighbors.size()];
				cell->Link(index);
			}
			});
		return grid;
	}

}
