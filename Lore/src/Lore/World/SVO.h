#pragma once

#include "Lore/World/VoxelGrid.h"

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

	// Resumen de un bloque cúbico del grid (un nodo potencial del octree).
	struct OctantSummary {
		uint32_t avgMaterial = 0;     // Color promedio del subárbol (hoja: su material; aire: 0)
		uint32_t uniformMaterial = 0; // Material si todo el bloque es igual (0 = aire)
		uint32_t solidCount = 0;      // Vóxeles sólidos en el bloque
		uint32_t isUniform = 0;       // 1 si todos los vóxeles del bloque son iguales

		bool IsAir() const { return isUniform && uniformMaterial == 0; }
		bool IsSolidLeaf() const { return isUniform && uniformMaterial != 0; }
	};

	// Construye el SVO de abajo hacia arriba:
	//
	//   1. Pirámide de resúmenes: el nivel 1 se calcula leyendo cada vóxel del grid UNA vez
	//      (en paralelo con el JobSystem) y cada nivel superior combina los 8 resúmenes del
	//      nivel de abajo. Costo O(N) en vez del O(N log N) de la versión recursiva anterior,
	//      que volvía a escanear el bloque completo en cada nivel (~9 pasadas sobre 16.7 M
	//      vóxeles para un mundo de 256^3).
	//   2. Emisión de nodos de arriba hacia abajo recorriendo sólo la pirámide (proporcional
	//      al número de nodos, no de vóxeles), en el mismo orden en profundidad que antes:
	//      el arreglo resultante es idéntico byte a byte al del constructor recursivo
	//      (Tests/ lo verifica), así que el shader no cambia.
	class SVOBuilder {
	public:
		// El tamaño del grid debe ser potencia de 2 y <= 1024. Si progress pide cancelar
		// devuelve un vector vacío.
		std::vector<SVONode> Build(const VoxelGrid& grid, const ProgressFn& progress = {});

	private:
		// m_Levels[L - 1] es el nivel L (bloques de 2^L vóxeles por lado). El nivel 0 es el
		// propio grid y no se copia.
		std::vector<std::vector<OctantSummary>> m_Levels;
		std::vector<SVONode> m_Nodes;
		const VoxelGrid* m_Grid = nullptr;

		OctantSummary GetSummary(uint32_t level, uint32_t x, uint32_t y, uint32_t z) const;
		void Emit(uint32_t nodeIndex, uint32_t level, uint32_t x, uint32_t y, uint32_t z);
	};
}
