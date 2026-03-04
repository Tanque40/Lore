#include "Maze/Base/Cell3D.h"

namespace Maze {

	std::vector<Cell*> Cell3D::GetNeighbors() {
		std::vector<Cell*> neighbors = Cell::GetNeighbors();
		if (m_Up) neighbors.push_back(m_Up);
		if (m_Down) neighbors.push_back(m_Down);
		return neighbors;
	}

}
