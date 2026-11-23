#pragma once

#include "Lore/ProcGen/Maze/Base/Cell3D.h"
#include "Lore/ProcGen/Maze/Base/Random.h"

#include <memory>

namespace Maze {

	class Grid3D {
	private:
		uint32_t m_Rows, m_Columns, m_Levels;
		std::vector<std::vector<std::vector<Cell3D*>>> m_Grid;
		// Dueño real de las celdas (ver Grid::m_Cells): las copias de Grid3D comparten las
		// celdas y la memoria se libera con la última copia.
		std::shared_ptr<std::vector<std::unique_ptr<Cell3D>>> m_Cells;

	public:
		Grid3D(uint32_t rows, uint32_t columns, uint32_t levels) : m_Rows(rows), m_Columns(columns), m_Levels(levels) {
			ConfigureCells();
		}

		inline uint32_t GetRows() { return m_Rows; }
		inline uint32_t GetColumns() { return m_Columns; }
		inline uint32_t GetLevels() { return m_Levels; }

		void PrepareGrid();
		void ConfigureCells();
		Cell3D* RandomCell(Random& rng = Random::Global());
		void EachLevel(std::function<void(std::vector<std::vector<Cell3D*>>)> callback);
		void EachRow(std::function<void(std::vector<Cell3D*>)> callback);
		void EachCell(std::function<void(Cell3D*)> callback);

		inline uint32_t Size() {
			return m_Rows * m_Columns * m_Levels;
		}

		// El orden de almacenamiento es m_Grid[level][row][column] (ver PrepareGrid); los
		// parametros siguen en el orden (row, column, level) por comodidad de llamada.
		Cell3D* operator ()(uint32_t row, uint32_t column, uint32_t level) {
			if (row < m_Rows && column < m_Columns && level < m_Levels)
				return m_Grid[level][row][column];

			return nullptr;
		}

		std::vector<std::vector<std::vector<uint32_t>>> ToIntMatrix3D();
		static std::string IntMatrix3DToString(const std::vector<std::vector<std::vector<uint32_t>>>& matrix3D);

		std::string ToString();
	};

}
