#include "sndbxpch.h"

#include "Maze/Algorithms/Wilsons.h"

namespace Maze {

	Grid Wilsons::On(Grid grid) {
		std::vector<Cell*> unvisited;
		grid.EachCell([&](Cell* cell) {
			unvisited.push_back(cell);
			});

		Cell* first = unvisited[std::rand() % unvisited.size()];
		unvisited.erase(std::remove(unvisited.begin(), unvisited.end(), first), unvisited.end());

		while (!unvisited.empty()) {
			Cell* cell = unvisited[std::rand() % unvisited.size()];
			std::vector<Cell*> path{ cell };

			while (std::find(unvisited.begin(), unvisited.end(), cell) != unvisited.end()) {
				std::vector<Cell*> neighbors = cell->GetNeighbors();
				cell = neighbors[std::rand() % neighbors.size()];

				auto it = std::find(path.begin(), path.end(), cell);
				if (it != path.end()) {
					path.erase(it + 1, path.end());
				}
				else {
					path.push_back(cell);
				}
			}

			for (size_t i = 0; i < path.size() - 1; ++i) {
				path[i]->Link(path[i + 1]);
				unvisited.erase(std::remove(unvisited.begin(), unvisited.end(), path[i]), unvisited.end());
			}
			unvisited.erase(std::remove(unvisited.begin(), unvisited.end(), path.back()), unvisited.end());
		}

		return grid;
	}

	// Wilson's no tiene direcciones privilegiadas (Este/Norte/etc): solo camina
	// por lo que devuelva GetNeighbors() de la celda actual y borra bucles del
	// camino. Cell3D::GetNeighbors() ya suma Arriba/Abajo a los 4 vecinos base,
	// asi que la misma caminata aleatoria con borrado de bucles funciona sin
	// cambios sobre el grafo 3D: sigue siendo un arbol de expansion muestreado
	// de forma uniforme entre todas las celdas, ahora en las 6 direcciones.
	void Wilsons::On(Grid3D* grid) {
		std::vector<Cell3D*> unvisited;
		grid->EachCell([&](Cell3D* cell) {
			unvisited.push_back(cell);
			});

		Cell3D* first = unvisited[std::rand() % unvisited.size()];
		unvisited.erase(std::remove(unvisited.begin(), unvisited.end(), first), unvisited.end());

		while (!unvisited.empty()) {
			Cell3D* cell = unvisited[std::rand() % unvisited.size()];
			std::vector<Cell3D*> path{ cell };

			while (std::find(unvisited.begin(), unvisited.end(), cell) != unvisited.end()) {
				std::vector<Cell*> neighbors = cell->GetNeighbors();
				cell = (Cell3D*)neighbors[std::rand() % neighbors.size()];

				auto it = std::find(path.begin(), path.end(), cell);
				if (it != path.end()) {
					path.erase(it + 1, path.end());
				}
				else {
					path.push_back(cell);
				}
			}

			for (size_t i = 0; i < path.size() - 1; ++i) {
				path[i]->Link(path[i + 1]);
				unvisited.erase(std::remove(unvisited.begin(), unvisited.end(), path[i]), unvisited.end());
			}
			unvisited.erase(std::remove(unvisited.begin(), unvisited.end(), path.back()), unvisited.end());
		}
	}

}
