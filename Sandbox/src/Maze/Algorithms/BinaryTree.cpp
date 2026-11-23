#include "sndbxpch.h"

#include "Maze/Algorithms/BinaryTree.h"

#include "Maze/Base/Cell.h"

namespace Maze {

	Grid BinaryTree::On(Grid grid, Random& rng) {
		grid.EachCell([&](Cell* cell) {
			std::vector<Cell*> neighbors;
			if (cell->GetNorth()) {
				neighbors.push_back(cell->GetNorth());
			}
			if (cell->GetEast()) {
				neighbors.push_back(cell->GetEast());
			}

			if (!neighbors.empty()) {
				Cell* index = neighbors[rng.Range(static_cast<uint32_t>(neighbors.size()))];
				cell->Link(index);
			}
			});
		return grid;
	}

	void BinaryTree::On(Grid3D* grid, Random& rng) {
		grid->EachCell([&](Cell3D* cell) {
			std::vector<Cell3D*> neighbors;
			if (cell->GetNorth()) {
				neighbors.push_back((Cell3D*)cell->GetNorth());
			}
			if (cell->GetEast()) {
				neighbors.push_back((Cell3D*)cell->GetEast());
			}
			if (cell->GetUp()) {
				neighbors.push_back(cell->GetUp());
			}

			if (!neighbors.empty()) {
				Cell3D* index = neighbors[rng.Range(static_cast<uint32_t>(neighbors.size()))];
				cell->Link(index);
			}
			});
	}

}
