#include "sndbxpch.h"

#include "Maze/Base/Grid3D.h"

namespace Maze {

	void Grid3D::PrepareGrid() {
		m_Grid.clear();
		for (uint32_t level = 0; level < m_Levels; level++) {
			std::vector<std::vector<Cell3D*>> gridLevel;
			for (uint32_t row = 0; row < m_Rows; row++) {
				std::vector<Cell3D*> gridRow;
				for (uint32_t column = 0; column < m_Columns; column++) {
					gridRow.push_back(new Cell3D(row, column, level));
				}
				gridLevel.push_back(gridRow);
			}
			m_Grid.push_back(gridLevel);
		}
	}

	void Grid3D::ConfigureCells() {
		PrepareGrid();

		EachCell([&](Cell3D* cell) {
			uint32_t row = cell->GetRow();
			uint32_t column = cell->GetColumn();
			uint32_t level = cell->GetLevel();

			if (row > 0) {
				cell->SetNorth(m_Grid[row - 1][column][level]);
			}
			if (row < m_Rows - 1) {
				cell->SetSouth(m_Grid[row + 1][column][level]);
			}
			if (column < m_Columns - 1) {
				cell->SetEast(m_Grid[row][column + 1][level]);
			}
			if (column > 0) {
				cell->SetWest(m_Grid[row][column - 1][level]);
			}
			if (level < m_Levels - 1) {
				cell->SetUp(m_Grid[row][column][level + 1]);
			}
			if (level > 0) {
				cell->SetDown(m_Grid[row][column][level - 1]);
			}
			});
	}

	Cell3D* Grid3D::RandomCell() {
		uint32_t row = std::rand() % m_Rows;
		uint32_t column = std::rand() % m_Columns;
		uint32_t level = std::rand() % m_Levels;
		return m_Grid[row][column][level];
	}

	void Grid3D::EachLevel(std::function<void(std::vector<std::vector<Cell3D*>>)> callback) {
		for (uint32_t level = 0; level < m_Levels; level++) {
			callback(m_Grid[level]);
		}
	}

	void Grid3D::EachRow(std::function<void(std::vector<Cell3D*>)> callback) {
		for (uint32_t level = 0; level < m_Levels; level++) {
			for (uint32_t row = 0; row < m_Rows; row++) {
				callback(m_Grid[level][row]);
			}
		}
	}

	void Grid3D::EachCell(std::function<void(Cell3D*)> callback) {
		for (uint32_t level = 0; level < m_Levels; level++) {
			for (uint32_t row = 0; row < m_Rows; row++) {
				for (uint32_t column = 0; column < m_Columns; column++) {
					callback(m_Grid[level][row][column]);
				}
			}
		}
	}


	std::string Grid3D::IntMatrix3DToString(const std::vector<std::vector<std::vector<uint32_t>>>& matrix3D) {
		std::string output = "";

		for (uint32_t level = 0; level < matrix3D.size(); level++) {
			output += "Level " + std::to_string(level) + "\n";
			for (auto row : matrix3D[level]) {
				for (auto cell : row) {
					output += std::to_string(cell) + " ";
				}
				output += "\n";
			}
			output += "\n";
		}

		return output;
	}

	std::string Grid3D::ToString() {
		std::string output = "";

		for (uint32_t level = 0; level < m_Levels; level++) {
			output += "Level " + std::to_string(level) + "\n";
			output += "+";
			for (unsigned int c = 0; c < m_Columns; c++) {
				output += "---+";
			}
			output += "\n";

			std::string body = "";

			for (unsigned int r = 0; r < m_Rows; r++) {
				std::string top = "|";
				std::string bottom = "+";
				for (unsigned int c = 0; c < m_Columns; c++) {
					Cell3D* cell = m_Grid[level][r][c];

					body += cell->IsLinked(cell->GetUp()) ? "U" : " ";
					body += " ";
					body += cell->IsLinked(cell->GetDown()) ? "D" : " ";

					std::string east_boundary = cell->IsLinked(cell->GetEast()) ? " " : "|";
					top += body + east_boundary;

					std::string south_boundary = cell->IsLinked(cell->GetSouth()) ? "   " : "---";
					bottom += south_boundary + "+";

					body = "";
				}
				output += top + "\n";
				output += bottom + "\n";
			}

		}

		return output;
	}

}
