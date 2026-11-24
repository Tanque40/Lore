// Pruebas de los generadores de laberintos (2D y 3D).
//
// Un laberinto "perfecto" es un árbol de expansión de la cuadrícula: todas las celdas
// alcanzables, exactamente (celdas - 1) pasillos, sin ciclos, y cada pasillo une dos celdas
// geométricamente adyacentes. Estas pruebas verifican esas propiedades para cada algoritmo
// y, además, que la misma semilla produzca exactamente el mismo laberinto.
//
#include "lrpch.h"

#include "TestCommon.h"

#include <queue>
#include <set>

#include "Lore/ProcGen/Maze/Base/Grid.h"
#include "Lore/ProcGen/Maze/Base/Grid3D.h"
#include "Lore/ProcGen/Maze/Algorithms/BinaryTree.h"
#include "Lore/ProcGen/Maze/Algorithms/Sidewinder.h"
#include "Lore/ProcGen/Maze/Algorithms/Wilsons.h"

namespace {

	uint32_t Distance(Maze::Cell* a, Maze::Cell* b) {
		auto diff = [](uint32_t x, uint32_t y) { return x > y ? x - y : y - x; };
		return diff(a->GetRow(), b->GetRow()) + diff(a->GetColumn(), b->GetColumn());
	}

	uint32_t Distance(Maze::Cell3D* a, Maze::Cell3D* b) {
		auto diff = [](uint32_t x, uint32_t y) { return x > y ? x - y : y - x; };
		return diff(a->GetRow(), b->GetRow()) + diff(a->GetColumn(), b->GetColumn()) + diff(a->GetLevel(), b->GetLevel());
	}

	// Verifica que las celdas formen un árbol de expansión con enlaces sólo entre vecinas.
	template<typename CellT>
	void CheckPerfectMaze(const std::vector<CellT*>& cells, const std::string& label) {
		size_t linkEnds = 0;
		size_t nonAdjacent = 0;
		size_t asymmetric = 0;
		for (CellT* cell : cells) {
			for (Maze::Cell* link : cell->GetLinks()) {
				linkEnds++;
				if (Distance(cell, static_cast<CellT*>(link)) != 1) nonAdjacent++;
				if (!link->IsLinked(cell)) asymmetric++;
			}
		}

		std::set<Maze::Cell*> seen{ cells.front() };
		std::queue<Maze::Cell*> pending;
		pending.push(cells.front());
		while (!pending.empty()) {
			Maze::Cell* cell = pending.front();
			pending.pop();
			for (Maze::Cell* link : cell->GetLinks())
				if (seen.insert(link).second) pending.push(link);
		}

		Check(linkEnds / 2 == cells.size() - 1, label + ": pasillos = celdas - 1 (" + std::to_string(linkEnds / 2) + " vs " + std::to_string(cells.size() - 1) + ")");
		Check(seen.size() == cells.size(), label + ": todas las celdas alcanzables (" + std::to_string(seen.size()) + "/" + std::to_string(cells.size()) + ")");
		Check(nonAdjacent == 0, label + ": " + std::to_string(nonAdjacent) + " enlaces entre celdas no adyacentes");
		Check(asymmetric == 0, label + ": " + std::to_string(asymmetric) + " enlaces no bidireccionales");
	}

	// Verifica que los punteros Norte/Sur/Este/Oeste/Arriba/Abajo apunten a la vecina correcta.
	void CheckNeighbors3D(Maze::Grid3D& grid, const std::string& label) {
		size_t wrong = 0;
		grid.EachCell([&](Maze::Cell3D* c) {
			uint32_t r = c->GetRow(), col = c->GetColumn(), l = c->GetLevel();
			if (c->GetNorth() != (r > 0 ? grid(r - 1, col, l) : nullptr)) wrong++;
			if (c->GetSouth() != grid(r + 1, col, l)) wrong++;
			if (c->GetEast() != grid(r, col + 1, l)) wrong++;
			if (c->GetWest() != (col > 0 ? grid(r, col - 1, l) : nullptr)) wrong++;
			if (c->GetUp() != grid(r, col, l + 1)) wrong++;
			if (c->GetDown() != (l > 0 ? grid(r, col, l - 1) : nullptr)) wrong++;
			if (grid(r, col, l) != c) wrong++;
			});
		Check(wrong == 0, label + ": " + std::to_string(wrong) + " vecinos mal asignados");
	}

	std::vector<Maze::Cell*> Cells(Maze::Grid& grid) {
		std::vector<Maze::Cell*> cells;
		grid.EachCell([&](Maze::Cell* c) { cells.push_back(c); });
		return cells;
	}

	std::vector<Maze::Cell3D*> Cells(Maze::Grid3D& grid) {
		std::vector<Maze::Cell3D*> cells;
		grid.EachCell([&](Maze::Cell3D* c) { cells.push_back(c); });
		return cells;
	}

	// Firma textual de un laberinto 3D, para comparar dos generaciones con la misma semilla.
	std::string Signature(Maze::Grid3D& grid) {
		return grid.ToString();
	}

	using Algo2D = Maze::Grid(*)(Maze::Grid, Maze::Random&);
	using Algo3D = void(*)(Maze::Grid3D*, Maze::Random&);

}

void RunMazeTests() {
	const std::vector<std::pair<std::string, Algo2D>> algos2D = {
		{ "BinaryTree", &Maze::BinaryTree::On },
		{ "Sidewinder", &Maze::Sidewinder::On },
		{ "Wilsons", &Maze::Wilsons::On },
	};
	const std::vector<std::pair<std::string, Algo3D>> algos3D = {
		{ "BinaryTree3D", &Maze::BinaryTree::On },
		{ "Sidewinder3D", &Maze::Sidewinder::On },
		{ "Wilsons3D", &Maze::Wilsons::On },
	};

	// Tamaños cúbicos y no cúbicos: un grid no cúbico detecta errores de orden de índices
	// que un cubo esconde (o que hacen leer fuera de rango).
	const std::vector<std::array<uint32_t, 2>> sizes2D = { {{1, 1}}, {{2, 2}}, {{10, 10}}, {{7, 13}}, {{13, 7}} };
	const std::vector<std::array<uint32_t, 3>> sizes3D = { {{1, 1, 1}}, {{2, 2, 2}}, {{5, 5, 5}}, {{3, 6, 4}}, {{6, 3, 2}}, {{1, 8, 3}} };
	const std::vector<uint64_t> seeds = { 1, 42, 20261123 };

	for (const auto& [name, algo] : algos2D) {
		std::printf("%s\n", name.c_str());
		for (const auto& size : sizes2D) {
			for (uint64_t seed : seeds) {
				Maze::Random rng(seed);
				Maze::Grid grid = algo(Maze::Grid(size[0], size[1]), rng);
				CheckPerfectMaze(Cells(grid), name + " " + std::to_string(size[0]) + "x" + std::to_string(size[1]) + " semilla " + std::to_string(seed));
			}
		}
	}

	for (const auto& [name, algo] : algos3D) {
		std::printf("%s\n", name.c_str());
		for (const auto& size : sizes3D) {
			for (uint64_t seed : seeds) {
				std::string label = name + " " + std::to_string(size[0]) + "x" + std::to_string(size[1]) + "x" + std::to_string(size[2]) + " semilla " + std::to_string(seed);

				Maze::Grid3D grid(size[0], size[1], size[2]);
				CheckNeighbors3D(grid, label);

				Maze::Random rng(seed);
				algo(&grid, rng);
				CheckPerfectMaze(Cells(grid), label);

				Maze::Grid3D again(size[0], size[1], size[2]);
				Maze::Random rngAgain(seed);
				algo(&again, rngAgain);
				Check(Signature(grid) == Signature(again), label + ": la misma semilla debe dar el mismo laberinto");
			}
		}
	}
}
