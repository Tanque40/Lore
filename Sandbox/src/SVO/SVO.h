#pragma once

#include "VoxelGrid.h"

namespace SVO {

	// El nodo empaquetado para la GPU. childIndex va en su propio uint32 (en vez de
	// empaquetado en 16 bits dentro de descriptor) porque un octree de alta resolución
	// con superficie irregular (p. ej. una estalactita con detalle fino) puede superar
	// los 65,536 nodos fácilmente; con 16 bits el índice se envolvía silenciosamente y
	// corrompía el recorrido en la GPU sin ningún error visible.
	struct SVONode {
		uint32_t descriptor; // Bits 0-7: Valid, Bits 8-15: Leaf (bits 16-31 sin usar)
		uint32_t childIndex; // Índice del primer hijo (rango completo de 32 bits)
		uint32_t material;   // Tu información de color (RGBA o RGB + Propiedad)
	};

	struct OctantResult {
		bool isAir = false;
		bool isSolidLeaf = false;
		uint32_t leafMaterial = 0;
	};

	class SVOBuilder {
	private:
		std::vector<SVONode> m_Nodes;

	public:
		std::vector<SVONode> Build(const VoxelGrid& grid);

	private:
		OctantResult BuildRecursive(uint32_t nodeIndex, uint32_t x, uint32_t y, uint32_t z, uint32_t size, const VoxelGrid& grid);
	};
}
