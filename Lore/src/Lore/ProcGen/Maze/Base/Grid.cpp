#include "lrpch.h"

#include "Lore/ProcGen/Maze/Base/Grid.h"

namespace Maze {

	void Grid::PrepareGrid() {
		m_Cells = std::make_shared<std::vector<std::unique_ptr<Cell>>>();
		m_Cells->reserve(static_cast<size_t>(m_Rows) * m_Columns);
		m_Grid.assign(m_Rows, std::vector<Cell*>(m_Columns, nullptr));
		for (unsigned int r = 0; r < m_Rows; r++) {
			for (unsigned int c = 0; c < m_Columns; c++) {
				m_Cells->push_back(std::make_unique<Cell>(r, c));
				m_Grid[r][c] = m_Cells->back().get();
			}
		}
	}

	void Grid::ConfigureCells() {
		PrepareGrid();
		for (unsigned int r = 0; r < m_Rows; r++) {
			for (unsigned int c = 0; c < m_Columns; c++) {
				Cell* cell = m_Grid[r][c];
				if (r > 0) {
					cell->SetNorth(m_Grid[r - 1][c]);
				}
				if (r < m_Rows - 1) {
					cell->SetSouth(m_Grid[r + 1][c]);
				}
				if (c < m_Columns - 1) {
					cell->SetEast(m_Grid[r][c + 1]);
				}
				if (c > 0) {
					cell->SetWest(m_Grid[r][c - 1]);
				}
			}
		}
	}

	Cell* Grid::RandomCell(Random& rng) {
		unsigned int r = rng.Range(m_Rows);
		unsigned int c = rng.Range(m_Columns);
		return m_Grid[r][c];
	}

	void Grid::EachRow(std::function<void(std::vector<Cell*>)> callback) {
		std::vector<Cell*> cells;
		for (unsigned int r = 0; r < m_Rows; r++) {
			callback(m_Grid[r]);
		}
	}

	void Grid::EachCell(std::function<void(Cell*)> callback) {
		for (unsigned int r = 0; r < m_Rows; r++) {
			for (unsigned int c = 0; c < m_Columns; c++) {
				callback(m_Grid[r][c]);
			}
		}
	}

	std::string Grid::ToString() {
		std::string output = "+";
		for (unsigned int c = 0; c < m_Columns; c++) {
			output += "---+";
		}
		output += "\n";

		std::string body = "   ";

		for (unsigned int r = 0; r < m_Rows; r++) {
			std::string top = "|";
			std::string bottom = "+";
			for (unsigned int c = 0; c < m_Columns; c++) {
				Cell* cell = m_Grid[r][c];
				std::string east_boundary = cell->IsLinked(cell->GetEast()) ? " " : "|";
				top += body + east_boundary;

				std::string south_boundary = cell->IsLinked(cell->GetSouth()) ? "   " : "---";
				bottom += south_boundary + "+";
			}
			output += top + "\n";
			output += bottom + "\n";
		}

		return output;
	}

}
