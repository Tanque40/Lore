#pragma once

#include "Maze/Base/Cell.h"

namespace Maze {

	class Cell3D : public Cell {
	private:
		uint32_t m_Level = 0;
		Cell3D* m_Up = nullptr;
		Cell3D* m_Down = nullptr;

	public:
		Cell3D(uint32_t r, uint32_t c, uint32_t level = 0) : Cell(r, c), m_Level(level) {}

		inline void SetUp(Cell3D* cell) { m_Up = cell; }
		inline void SetDown(Cell3D* cell) { m_Down = cell; }
		inline void SetLevel(uint32_t level) { m_Level = level; }
		inline Cell3D* GetUp() { return m_Up; }
		inline Cell3D* GetDown() { return m_Down; }
		inline uint32_t GetLevel() { return m_Level; }

		std::vector<Cell*> GetNeighbors();
	};

}
