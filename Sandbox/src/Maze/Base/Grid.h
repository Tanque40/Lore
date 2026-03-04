#pragma once

#include "Maze/Base/Cell.h"

namespace Maze {

	class Grid {
	private:
		uint32_t m_Rows;
		uint32_t m_Columns;
		std::vector<std::vector<Cell*>> m_Grid;

	public:
		Grid(uint32_t r, uint32_t c) : m_Rows(r), m_Columns(c) {
			ConfigureCells();
		}
		void PrepareGrid();
		void ConfigureCells();
		Cell* RandomCell();
		void EachRow(std::function<void(std::vector<Cell*>)> callback);
		void EachCell(std::function<void(Cell*)> callback);

		inline uint32_t Size() {
			return m_Rows * m_Columns;
		}

		Cell* operator ()(uint32_t row, uint32_t column) {
			if (0 <= row && row < m_Rows && 0 <= column && column < m_Columns)
				return m_Grid[row][column];

			return nullptr;
		}

		std::string ToString();
	};

}
