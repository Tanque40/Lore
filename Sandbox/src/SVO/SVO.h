#pragma once

#include "VoxelGrid.h"

namespace SVO {

	// El nodo empaquetado para la GPU. childIndex va en su propio uint32 (en vez de
	// empaquetado en 16 bits dentro de descriptor) porque un octree de alta resolución
	// con superficie irregular (p. ej. una estalactita con detalle fino) puede superar
	// los 65,536 nodos fácilmente; con 16 bits el índice se envolvía silenciosamente y
	// corrompía el recorrido en la GPU sin ningún error visible.
	struct SVONode {
		// Bits 0-7: Valid, Bits 8-15: Leaf, Bits 16-23: Coverage (bits 24-31 sin usar).
		// Coverage es la fracción (0-255) de vóxeles sólidos en TODO el subárbol de este
		// nodo, no sólo de sus hijos directos; junto con material (que para un nodo rama
		// guarda el color PROMEDIO de ese subárbol, no un color real) le permite al ray
		// marcher de la GPU renderizar el nodo como un único bloque "impostor" cuando está
		// lo bastante lejos como para no necesitar bajar hasta el detalle exacto (LOD).
		uint32_t descriptor;
		uint32_t childIndex; // Índice del primer hijo (rango completo de 32 bits)
		uint32_t material;   // Hoja: color real. Rama: color promedio ponderado del subárbol.
	};

	struct OctantResult {
		bool isAir = false;
		bool isSolidLeaf = false;
		uint32_t leafMaterial = 0;
		uint32_t avgMaterial = 0;  // Color promedio del subárbol completo (hoja o rama)
		uint64_t solidCount = 0;   // Vóxeles sólidos en el subárbol, para ponderar el promedio al subir un nivel
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
