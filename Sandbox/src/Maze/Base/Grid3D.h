#pragma once

#include "Maze/Base/Cell3D.h"

namespace Maze {

	class Grid3D {
	private:
		uint32_t m_Rows, m_Columns, m_Levels;
		std::vector<std::vector<std::vector<Cell3D*>>> m_Grid;

	public:
		Grid3D(uint32_t rows, uint32_t columns, uint32_t levels) : m_Rows(rows), m_Columns(columns), m_Levels(levels) {
			ConfigureCells();
		}

		inline uint32_t GetLevels() { return m_Levels; }

		void PrepareGrid();
		void ConfigureCells();
		Cell3D* RandomCell();
		void EachLevel(std::function<void(std::vector<std::vector<Cell3D*>>)> callback);
		void EachRow(std::function<void(std::vector<Cell3D*>)> callback);
		void EachCell(std::function<void(Cell3D*)> callback);

		inline uint32_t Size() {
			return m_Rows * m_Columns * m_Levels;
		}

		Cell3D* operator ()(uint32_t row, uint32_t column, uint32_t level) {
			if (0 <= row && row < m_Rows && 0 <= column && column < m_Columns && 0 <= level && level < m_Levels)
				return m_Grid[row][column][level];

			return nullptr;
		}

		std::vector<std::vector<std::vector<uint32_t>>> ToIntMatrix3D();
		static std::string IntMatrix3DToString(const std::vector<std::vector<std::vector<uint32_t>>>& matrix3D);

		std::string ToString();
	};

}
