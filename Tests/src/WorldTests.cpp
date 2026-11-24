#include "lrpch.h"

#include "TestCommon.h"

#include <chrono>

#include "Lore/World/SVO.h"
#include "Lore/Core/JobSystem.h"
#include "Lore/World/VoxelGrid.h"
#include "Lore/ProcGen/Maze/Base/Grid3D.h"
#include "Lore/ProcGen/Maze/Algorithms/Wilsons.h"

namespace {

	// =====================================================================================
	// Constructor de referencia: copia del SVOBuilder recursivo original (antes de la
	// Fase 1). El nuevo tiene que producir EXACTAMENTE el mismo arreglo de nodos, porque el
	// shader depende del orden y del contenido de cada nodo.
	// =====================================================================================
	class ReferenceSVOBuilder {
	private:
		struct Result {
			bool isAir = false;
			bool isSolidLeaf = false;
			uint32_t avgMaterial = 0;
			uint64_t solidCount = 0;
		};

		std::vector<SVO::SVONode> m_Nodes;

		static void UnpackRGB(uint32_t m, uint32_t& r, uint32_t& g, uint32_t& b) {
			r = m & 0xFF; g = (m >> 8) & 0xFF; b = (m >> 16) & 0xFF;
		}

		Result BuildRecursive(uint32_t nodeIndex, uint32_t x, uint32_t y, uint32_t z, uint32_t size, const SVO::VoxelGrid& grid) {
			bool isAllAir = true;
			bool isAllSolid = true;
			uint32_t firstVoxel = grid.GetVoxel(x, y, z);
			for (uint32_t dz = 0; dz < size; dz++)
				for (uint32_t dy = 0; dy < size; dy++)
					for (uint32_t dx = 0; dx < size; dx++) {
						uint32_t v = grid.GetVoxel(x + dx, y + dy, z + dz);
						if (v != 0) isAllAir = false;
						if (v != firstVoxel) isAllSolid = false;
					}

			if (isAllAir) return { true, false, 0, 0 };
			if (isAllSolid && firstVoxel != 0) {
				m_Nodes[nodeIndex].material = firstVoxel;
				return { false, true, firstVoxel, static_cast<uint64_t>(size) * size * size };
			}

			uint32_t half = size / 2;
			uint32_t base = static_cast<uint32_t>(m_Nodes.size());
			for (int i = 0; i < 8; ++i) m_Nodes.push_back({ 0, 0, 0 });

			uint8_t validMask = 0, leafMask = 0;
			uint64_t total = 0, rSum = 0, gSum = 0, bSum = 0;
			for (int i = 0; i < 8; ++i) {
				Result child = BuildRecursive(base + i, x + ((i & 1) ? half : 0), y + ((i & 2) ? half : 0), z + ((i & 4) ? half : 0), half, grid);
				if (!child.isAir) validMask |= (1 << i);
				if (child.isSolidLeaf) leafMask |= (1 << i);
				if (child.solidCount > 0) {
					uint32_t r, g, b;
					UnpackRGB(child.avgMaterial, r, g, b);
					rSum += r * child.solidCount; gSum += g * child.solidCount; bSum += b * child.solidCount;
					total += child.solidCount;
				}
			}

			uint32_t descriptor = validMask | (leafMask << 8);
			uint32_t avg = 0;
			if (total > 0) {
				avg = 0xFF000000u | (static_cast<uint32_t>(bSum / total) << 16) | (static_cast<uint32_t>(gSum / total) << 8) | static_cast<uint32_t>(rSum / total);
				uint64_t totalVoxels = static_cast<uint64_t>(size) * size * size;
				descriptor |= ((static_cast<uint32_t>((total * 255ull) / totalVoxels) & 0xFF) << 16);
			}
			m_Nodes[nodeIndex] = { descriptor, base, avg };
			return { false, false, avg, total };
		}

	public:
		std::vector<SVO::SVONode> Build(const SVO::VoxelGrid& grid) {
			m_Nodes.clear();
			m_Nodes.push_back({ 0, 0, 0 });
			BuildRecursive(0, 0, 0, 0, grid.GetSize(), grid);
			return m_Nodes;
		}
	};

	bool SameNodes(const std::vector<SVO::SVONode>& a, const std::vector<SVO::SVONode>& b, std::string& detail) {
		if (a.size() != b.size()) {
			detail = "tamaños " + std::to_string(a.size()) + " vs " + std::to_string(b.size());
			return false;
		}
		for (size_t i = 0; i < a.size(); i++) {
			if (a[i].descriptor != b[i].descriptor || a[i].childIndex != b[i].childIndex || a[i].material != b[i].material) {
				detail = "primer nodo distinto: " + std::to_string(i);
				return false;
			}
		}
		detail = std::to_string(a.size()) + " nodos";
		return true;
	}

	void CompareBuilders(const SVO::VoxelGrid& grid, const std::string& label) {
		ReferenceSVOBuilder reference;
		SVO::SVOBuilder builder;
		std::string detail;
		bool same = SameNodes(reference.Build(grid), builder.Build(grid), detail);
		Check(same, "SVO bottom-up == recursivo: " + label + " (" + detail + ")");
		std::printf("  %s: %s\n", label.c_str(), detail.c_str());
	}

	SVO::VoxelGrid MazeWorld(uint32_t cells, uint32_t seed, uint32_t gridSize) {
		Maze::Grid3D maze(cells, cells, cells);
		Maze::Random rng(seed);
		Maze::Wilsons::On(&maze, rng);
		return SVO::VoxelGrid::CastToVoxels(&maze, static_cast<int>(gridSize));
	}

	double Seconds(std::chrono::steady_clock::time_point start) {
		return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
	}

}

void RunWorldTests(bool benchmark) {
	// Casos borde
	{
		SVO::VoxelGrid empty(16);
		CompareBuilders(empty, "todo aire 16^3");

		SVO::VoxelGrid full(16);
		full.Fill(0xFF808080);
		CompareBuilders(full, "todo sólido 16^3");

		SVO::VoxelGrid single(8);
		single.SetVoxel(3, 5, 6, 0xFFFFFFFF); // Material con todos los bits encendidos
		CompareBuilders(single, "un vóxel blanco 8^3");

		SVO::VoxelGrid one(1);
		one.SetVoxel(0, 0, 0, 0xFF00FF00);
		CompareBuilders(one, "grid 1^3");
	}

	// Ruido de colores aleatorios con densidades distintas (muchas ramas, promedios de LOD).
	for (float density : { 0.05f, 0.5f, 0.95f }) {
		SVO::VoxelGrid noise(32);
		std::mt19937 gen(1234);
		std::uniform_real_distribution<float> coin(0.0f, 1.0f);
		std::uniform_int_distribution<uint32_t> color(0, 0xFFFFFFFF);
		for (uint32_t z = 0; z < 32; z++)
			for (uint32_t y = 0; y < 32; y++)
				for (uint32_t x = 0; x < 32; x++)
					if (coin(gen) < density) noise.SetVoxel(x, y, z, color(gen) | 1u);
		CompareBuilders(noise, "ruido " + std::to_string(static_cast<int>(density * 100)) + "% 32^3");
	}

	// Mundos reales: cueva sobre un laberinto y un espeleotema.
	CompareBuilders(MazeWorld(2, 7, 32), "cueva Wilson 2^3 en 32^3");
	CompareBuilders(MazeWorld(5, 1, 64), "cueva Wilson 5^3 en 64^3");
	CompareBuilders(SVO::VoxelGrid::CreateStalactite(64), "estalactita 64^3");

	// Cancelación: un callback que devuelve false corta la construcción.
	{
		SVO::VoxelGrid grid = MazeWorld(2, 7, 32);
		SVO::SVOBuilder builder;
		float last = -1.0f;
		std::vector<SVO::SVONode> nodes = builder.Build(grid, [&last](float p) { last = p; return false; });
		Check(nodes.empty(), "SVOBuilder cancelado devuelve vacío");

		bool monotonic = true;
		float previous = -1.0f;
		builder.Build(grid, [&](float p) { monotonic = monotonic && p >= previous; previous = p; return true; });
		Check(monotonic && previous == 1.0f, "SVOBuilder reporta progreso creciente hasta 1");

		Maze::Grid3D maze(3, 3, 3);
		Maze::Random rng(3);
		Maze::Wilsons::On(&maze, rng);
		SVO::VoxelGrid cancelled = SVO::VoxelGrid::CastToVoxels(&maze, 32, [](float) { return false; });
		Check(cancelled.GetSize() == 0, "CastToVoxels cancelado devuelve un grid vacío");
	}

	if (benchmark) {
		struct Case { std::string name; std::function<SVO::VoxelGrid()> make; };
		const std::vector<Case> cases = {
			{ "cueva Wilson 5^3 (256^3)", [] { return MazeWorld(5, 1, 256); } },
			{ "cueva Wilson 20^3 (256^3)", [] { return MazeWorld(20, 1, 256); } },
			{ "estalactita 256^3", [] { return SVO::VoxelGrid::CreateStalactite(256); } },
		};
		for (const Case& c : cases) {
			std::printf("  -- benchmark: %s --\n", c.name.c_str());
			auto start = std::chrono::steady_clock::now();
			SVO::VoxelGrid grid = c.make();
			std::printf("  generar vóxeles: %.3f s\n", Seconds(start));

			start = std::chrono::steady_clock::now();
			std::vector<SVO::SVONode> before = ReferenceSVOBuilder().Build(grid);
			double recursive = Seconds(start);

			start = std::chrono::steady_clock::now();
			std::vector<SVO::SVONode> after = SVO::SVOBuilder().Build(grid);
			double bottomUp = Seconds(start);

			std::string detail;
			Check(SameNodes(before, after, detail), "SVO bottom-up == recursivo, " + c.name + " (" + detail + ")");
			std::printf("  SVO recursivo: %.3f s | bottom-up: %.3f s (%u hilos) | %.1fx | %s\n",
				recursive, bottomUp, Lore::JobSystem::GetWorkerCount() + 1, recursive / bottomUp, detail.c_str());
		}
	}
}
