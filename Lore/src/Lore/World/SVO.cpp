#include "lrpch.h"

#include "Lore/World/SVO.h"

#include "Lore/Core/JobSystem.h"

namespace {

	// Mismo layout de bits que DecodificarColor en los shaders: r=bits0-7, g=bits8-15,
	// b=bits16-23, a=bits24-31.
	inline void UnpackRGB(uint32_t material, uint32_t& r, uint32_t& g, uint32_t& b) {
		r = material & 0xFF;
		g = (material >> 8) & 0xFF;
		b = (material >> 16) & 0xFF;
	}

	inline uint32_t PackRGB(uint32_t r, uint32_t g, uint32_t b) {
		// Alpha siempre opaco: sólo se llama con contenido sólido de por medio.
		return 0xFF000000u | (b << 16) | (g << 8) | r;
	}

	inline SVO::OctantSummary VoxelSummary(uint32_t voxel) {
		return { voxel, voxel, voxel != 0 ? 1u : 0u, 1u };
	}

	// Combina los 8 hijos (en orden Morton: bit0 = x, bit1 = y, bit2 = z) en el resumen del
	// padre. El promedio se calcula igual que la versión recursiva (promedio ponderado de
	// los promedios de los hijos, con división entera) para que los colores del LOD salgan
	// idénticos.
	inline SVO::OctantSummary Combine(const SVO::OctantSummary (&children)[8]) {
		bool uniform = true;
		for (int i = 1; i < 8; i++) {
			if (!children[i].isUniform || children[i].uniformMaterial != children[0].uniformMaterial) {
				uniform = false;
				break;
			}
		}
		uniform = uniform && children[0].isUniform;

		uint64_t solid = 0;
		uint64_t rSum = 0, gSum = 0, bSum = 0;
		for (const SVO::OctantSummary& child : children) {
			if (child.solidCount == 0) continue;
			uint32_t r, g, b;
			UnpackRGB(child.avgMaterial, r, g, b);
			rSum += static_cast<uint64_t>(r) * child.solidCount;
			gSum += static_cast<uint64_t>(g) * child.solidCount;
			bSum += static_cast<uint64_t>(b) * child.solidCount;
			solid += child.solidCount;
		}

		SVO::OctantSummary result;
		result.solidCount = static_cast<uint32_t>(solid);
		result.isUniform = uniform ? 1u : 0u;
		if (uniform) {
			result.uniformMaterial = children[0].uniformMaterial;
			result.avgMaterial = children[0].uniformMaterial;
		} else if (solid > 0) {
			result.avgMaterial = PackRGB(
				static_cast<uint32_t>(rSum / solid),
				static_cast<uint32_t>(gSum / solid),
				static_cast<uint32_t>(bSum / solid));
		}
		return result;
	}

}

namespace SVO {

	OctantSummary SVOBuilder::GetSummary(uint32_t level, uint32_t x, uint32_t y, uint32_t z) const {
		if (level == 0)
			return VoxelSummary(m_Grid->GetVoxel(x, y, z));
		uint32_t dim = m_Grid->GetSize() >> level;
		return m_Levels[level - 1][x + static_cast<size_t>(y) * dim + static_cast<size_t>(z) * dim * dim];
	}

	std::vector<SVONode> SVOBuilder::Build(const VoxelGrid& grid, const ProgressFn& progress) {
		m_Grid = &grid;
		m_Levels.clear();
		m_Nodes.clear();

		const uint32_t size = grid.GetSize();
		if (size == 0)
			return {};
		LR_CORE_ASSERT((size & (size - 1)) == 0, "SVOBuilder: el tamaño del grid debe ser potencia de 2");
		// solidCount es de 32 bits: 1024^3 = 2^30 vóxeles todavía cabe.
		LR_CORE_ASSERT(size <= 1024, "SVOBuilder: grid demasiado grande");

		uint32_t levelCount = 0;
		while ((1u << levelCount) < size) levelCount++;

		// ---------------------------------------------------------
		// PASO 1: Pirámide de resúmenes, de abajo hacia arriba
		// ---------------------------------------------------------
		const std::vector<uint32_t>& voxels = grid.GetData();
		m_Levels.resize(levelCount);

		for (uint32_t level = 1; level <= levelCount; level++) {
			const uint32_t dim = size >> level;
			const uint32_t childDim = dim * 2;
			std::vector<OctantSummary>& current = m_Levels[level - 1];
			current.resize(static_cast<size_t>(dim) * dim * dim);
			const std::vector<OctantSummary>* below = level > 1 ? &m_Levels[level - 2] : nullptr;

			// Una rebanada z por bloque: el nivel 1 de un mundo 256^3 son 128 rebanadas de
			// 128x128 resúmenes, de sobra para repartir entre todos los núcleos.
			Lore::JobSystem::ParallelFor(dim, 1, [&](uint32_t zBegin, uint32_t zEnd) {
				OctantSummary children[8];
				for (uint32_t z = zBegin; z < zEnd; z++)
					for (uint32_t y = 0; y < dim; y++)
						for (uint32_t x = 0; x < dim; x++) {
							for (uint32_t i = 0; i < 8; i++) {
								size_t cx = 2 * x + (i & 1);
								size_t cy = 2 * y + ((i >> 1) & 1);
								size_t cz = 2 * z + ((i >> 2) & 1);
								size_t index = cx + cy * childDim + cz * childDim * childDim;
								children[i] = below ? (*below)[index] : VoxelSummary(voxels[index]);
							}
							current[x + static_cast<size_t>(y) * dim + static_cast<size_t>(z) * dim * dim] = Combine(children);
						}
				});

			// El nivel 1 es ~7/8 del trabajo total; el resto es casi gratis.
			float done = level == 1 ? 0.85f : 0.85f + 0.1f * static_cast<float>(level - 1) / levelCount;
			if (!ReportProgress(progress, done)) {
				m_Levels.clear();
				return {};
			}
		}

		// ---------------------------------------------------------
		// PASO 2: Emitir nodos de arriba hacia abajo
		// ---------------------------------------------------------
		// Cada bloque mixto (ni aire puro ni hoja sólida) agrega exactamente 8 hijos: con la
		// pirámide ya sabemos el tamaño final y reservamos una sola vez.
		size_t mixed = 0;
		for (const std::vector<OctantSummary>& level : m_Levels)
			for (const OctantSummary& summary : level)
				if (!summary.isUniform) mixed++;
		m_Nodes.reserve(1 + mixed * 8);

		// Insertamos la raíz vacía en el índice 0
		m_Nodes.push_back({ 0, 0, 0 });
		Emit(0, levelCount, 0, 0, 0);

		m_Levels.clear();
		m_Levels.shrink_to_fit();
		ReportProgress(progress, 1.0f);
		return std::move(m_Nodes);
	}

	void SVOBuilder::Emit(uint32_t nodeIndex, uint32_t level, uint32_t x, uint32_t y, uint32_t z) {
		OctantSummary summary = GetSummary(level, x, y, z);

		// Casos base (poda del árbol): aire puro no escribe nada; un bloque sólido uniforme
		// es una hoja que sólo guarda su material.
		if (summary.IsAir())
			return;
		if (summary.IsSolidLeaf()) {
			m_Nodes[nodeIndex].material = summary.uniformMaterial;
			return;
		}

		// Nodo mezcla (rama): sus 8 hijos van contiguos a partir de baseChildIndex.
		uint32_t baseChildIndex = static_cast<uint32_t>(m_Nodes.size());
		for (int i = 0; i < 8; ++i)
			m_Nodes.push_back({ 0, 0, 0 });

		uint8_t validMask = 0;
		uint8_t leafMask = 0;
		for (uint32_t i = 0; i < 8; ++i) {
			uint32_t cx = 2 * x + (i & 1);
			uint32_t cy = 2 * y + ((i >> 1) & 1);
			uint32_t cz = 2 * z + ((i >> 2) & 1);

			OctantSummary child = GetSummary(level - 1, cx, cy, cz);
			if (!child.IsAir()) validMask |= (1 << i);
			if (child.IsSolidLeaf()) leafMask |= (1 << i);

			Emit(baseChildIndex + i, level - 1, cx, cy, cz);
		}

		uint32_t descriptor = 0;
		descriptor |= (validMask & 0xFF);       // Bits 0-7
		descriptor |= ((leafMask & 0xFF) << 8); // Bits 8-15
		if (summary.solidCount > 0) {
			uint64_t side = 1ull << level;
			uint64_t totalVoxels = side * side * side;
			uint32_t coverage = static_cast<uint32_t>((static_cast<uint64_t>(summary.solidCount) * 255ull) / totalVoxels);
			descriptor |= ((coverage & 0xFF) << 16); // Bits 16-23
		}

		m_Nodes[nodeIndex].descriptor = descriptor;
		m_Nodes[nodeIndex].childIndex = baseChildIndex; // Rango completo de 32 bits, sin truncar
		m_Nodes[nodeIndex].material = summary.avgMaterial; // Color promedio del subárbol, para el LOD
	}

}
