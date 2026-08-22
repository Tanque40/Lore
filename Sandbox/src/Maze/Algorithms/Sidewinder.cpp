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

	// Generalizacion a 3D: Grid3D::EachRow recorre nivel por nivel (de abajo hacia
	// arriba) y, dentro de cada nivel, fila por fila, exactamente el orden que
	// necesita esta prueba de arbol de expansion. Cada run se sigue armando hacia
	// el Este como en 2D, pero al cerrarse puede tallar hacia el Norte (fila ya
	// resuelta del mismo nivel) o hacia Abajo (nivel ya resuelto por completo),
	// nunca hacia Arriba: el nivel siguiente todavia no existe en el arbol cuando
	// se procesa el nivel actual, tallar hacia el todavia crearia un ciclo apenas
	// se lo visite mas tarde. Con como maximo un tallado hacia atras por run, cada
	// run (un camino) se suelda al arbol ya construido por una unica arista, asi
	// que el resultado sigue siendo un arbol de expansion perfecto (sin ciclos,
	// totalmente conexo) en vez de un grafo con loops.
	void Sidewinder::On(Grid3D* grid) {
		grid->EachRow([&](std::vector<Cell3D*> row) {
			std::vector<Cell3D*> run;

			for (Cell3D* cell : row) {
				run.push_back(cell);

				bool atEasternBoundary = (cell->GetEast() == nullptr);
				bool canCloseBackward = (cell->GetNorth() != nullptr) || (cell->GetDown() != nullptr);

				bool shouldCloseOut = atEasternBoundary || (canCloseBackward && (std::rand() % 2 == 0));

				if (shouldCloseOut) {
					Cell3D* member = run[std::rand() % run.size()];

					bool memberHasNorth = (member->GetNorth() != nullptr);
					bool memberHasDown = (member->GetDown() != nullptr);

					if (memberHasNorth && memberHasDown) {
						if (std::rand() % 2 == 0)
							member->Link(member->GetNorth());
						else
							member->Link(member->GetDown());
					}
					else if (memberHasNorth) {
						member->Link(member->GetNorth());
					}
					else if (memberHasDown) {
						member->Link(member->GetDown());
					}

					run.clear();
				}
				else {
					cell->Link(cell->GetEast());
				}
			}
			});
	}

}
