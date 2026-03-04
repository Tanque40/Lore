#include "sndbxpch.h"

#include "Maze/Algorithms/Sidewinder.h"

namespace Maze {

	Grid Sidewinder::On(Grid grid) {
		grid.EachRow([&](std::vector<Cell*> row) {
			std::vector<Cell*> run;

			for (Cell* cell : row) {
				run.push_back(cell);

				bool atEasternBoundary = (cell->GetEast() == nullptr);
				bool atNorthernBoundary = (cell->GetNorth() == nullptr);

				bool shouldCloseOut = atEasternBoundary || (!atNorthernBoundary && (std::rand() % 2 == 0));

				if (shouldCloseOut) {
					Cell* member = run[std::rand() % run.size()];
					if (member->GetNorth()) {
						member->Link(member->GetNorth());
					}
					run.clear();
				}
				else {
					cell->Link(cell->GetEast());
				}
			}
			});

		return grid;
	}

}
