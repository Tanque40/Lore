#pragma once

#include <map>
#include <vector>

namespace Maze {

	class Cell {
	private:
		unsigned int m_Row, m_Column;
		Cell* m_North = nullptr;
		Cell* m_South = nullptr;
		Cell* m_East = nullptr;
		Cell* m_West = nullptr;
		std::map<Cell*, bool> m_Links;

	public:
		Cell(unsigned int r, unsigned int c) : m_Row(r), m_Column(c) {}

		inline void SetNorth(Cell* cell) { m_North = cell; }
		inline void SetSouth(Cell* cell) { m_South = cell; }
		inline void SetEast(Cell* cell) { m_East = cell; }
		inline void SetWest(Cell* cell) { m_West = cell; }
		inline Cell* GetNorth() { return m_North; }
		inline Cell* GetSouth() { return m_South; }
		inline Cell* GetEast() { return m_East; }
		inline Cell* GetWest() { return m_West; }

		void Link(Cell* cell, bool bidirectional = true);
		void Unlink(Cell* cell, bool bidirectional = true);

		std::vector<Cell*> GetLinks();
		bool IsLinked(Cell* cell);
		std::vector<Cell*> GetNeighbors();
	};

}
