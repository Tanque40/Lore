#pragma once

#include "Maze/Base/Cell.h"
#include "Maze/Base/Random.h"

#include <memory>

namespace Maze {

	class Grid {
	private:
		uint32_t m_Rows;
		uint32_t m_Columns;
		std::vector<std::vector<Cell*>> m_Grid;
		// Dueño real de las celdas. m_Grid sólo guarda punteros de acceso rápido. Es
		// shared_ptr porque los algoritmos reciben y devuelven Grid por valor: las copias
		// comparten las mismas celdas (igual que antes) y la memoria se libera cuando se
		// destruye la última copia, en vez de fugarse en cada regeneración.
		std::shared_ptr<std::vector<std::unique_ptr<Cell>>> m_Cells;

	public:
		Grid(uint32_t r, uint32_t c) : m_Rows(r), m_Columns(c) {
			ConfigureCells();
		}
		void PrepareGrid();
		void ConfigureCells();
		Cell* RandomCell(Random& rng = Random::Global());
		void EachRow(std::function<void(std::vector<Cell*>)> callback);
		void EachCell(std::function<void(Cell*)> callback);

		inline uint32_t Size() {
			return m_Rows * m_Columns;
		}

		Cell* operator ()(uint32_t row, uint32_t column) {
			if (row < m_Rows && column < m_Columns)
				return m_Grid[row][column];

			return nullptr;
		}

		std::string ToString();
	};

}
