#include "sndbxpch.h"

#include "Maze/Base/Cell.h"

namespace Maze {

	void Cell::Link(Cell* cell, bool bidirectional) {
		m_Links[cell] = true;
		if (bidirectional) {
			cell->Link(this, false);
		}
	}

	void Cell::Unlink(Cell* cell, bool bidirectional) {
		m_Links.erase(cell);
		if (bidirectional) {
			cell->Unlink(this, false);
		}
	}

	std::vector<Cell*> Cell::GetLinks() {
		std::vector<Cell*> keys;
		for (const auto& pair : m_Links) {
			keys.push_back(pair.first);
		}
		return keys;
	}

	bool Cell::IsLinked(Cell* cell) {
		return m_Links.find(cell) != m_Links.end();
	}

	std::vector<Cell*> Cell::GetNeighbors() {
		std::vector<Cell*> list;

		if (m_North)	list.push_back(m_North);
		if (m_South)	list.push_back(m_South);
		if (m_East)	list.push_back(m_East);
		if (m_West)	list.push_back(m_West);

		return list;
	}

}
