#pragma once

#include "Maze/Base/Cell.h"

#include <vector>
#include <string>
#include <functional>

namespace Maze {

	class Grid {
	private:
		unsigned int m_Rows;
		unsigned int m_Columns;
		std::vector<std::vector<Cell*>> m_Grid;

	public:
		Grid(unsigned int r, unsigned int c) : m_Rows(r), m_Columns(c) {
			ConfigureCells();
		}
		void PrepareGrid();
		void ConfigureCells();
		Cell* RandomCell();
		void EachRow(std::function<void(std::vector<Cell*>)> callback);
		void EachCell(std::function<void(Cell*)> callback);

		inline unsigned int Size() {
			return m_Rows * m_Columns;
		}

		Cell* operator ()(unsigned int row, unsigned int column) {
			if (0 <= row && row < m_Rows && 0 <= column && column < m_Columns)
				return m_Grid[row][column];

			return nullptr;
		}

		std::string ToString();
	};

}
