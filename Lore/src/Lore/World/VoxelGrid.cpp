#include "lrpch.h"

#include "Lore/World/VoxelGrid.h"

namespace {

	// Convierte un tamaño en bytes a la unidad más legible (bytes/KB/MB/GB), escalando por
	// 1024 en cada paso. Por debajo de 1024 bytes se muestra el entero exacto sin decimales;
	// a partir de KB se usan 2 decimales para no perder precisión al redondear.
	std::string FormatBytesHumanReadable(uint64_t bytes) {
		static constexpr const char* kUnits[] = { "bytes", "KB", "MB", "GB" };
		constexpr size_t kUnitCount = sizeof(kUnits) / sizeof(kUnits[0]);

		if (bytes < 1024) {
			return std::to_string(bytes) + " " + kUnits[0];
		}

		double value = static_cast<double>(bytes);
		size_t unitIndex = 0;
		while (value >= 1024.0 && unitIndex + 1 < kUnitCount) {
			value /= 1024.0;
			unitIndex++;
		}

		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2) << value << " " << kUnits[unitIndex];
		return oss.str();
	}

	// Ruido de valor 3D con aritmética escalar simple. Los generadores de espeleotemas
	// (estalactitas/estalagmitas) necesitan millones de muestras de ruido para las estrías
	// de la superficie; glm::perlin (glm/gtc/noise.hpp) hace eso mismo pero encadenando
	// decenas de operadores vec4 con templates, que en un build sin optimizar (Debug) no se
	// inlinean y vuelven cada llamada órdenes de magnitud más lenta (una estalactita de
	// 256^3 tardaba ~18s en generarse). Esta versión es suficiente para dar textura
	// orgánica y es rápida incluso a -O0.
	inline float ValueNoiseHash(int x, int y, int z) {
		uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u + static_cast<uint32_t>(z) * 2147483647u;
		h = (h ^ (h >> 13)) * 1274126177u;
		h ^= (h >> 16);
		return static_cast<float>(h & 0xFFFFFFu) / static_cast<float>(0xFFFFFFu) * 2.0f - 1.0f;
	}

	inline float ValueNoiseSmoothStep(float t) { return t * t * (3.0f - 2.0f * t); }

	float ValueNoise3D(float x, float y, float z) {
		int x0 = static_cast<int>(std::floor(x)); int x1 = x0 + 1;
		int y0 = static_cast<int>(std::floor(y)); int y1 = y0 + 1;
		int z0 = static_cast<int>(std::floor(z)); int z1 = z0 + 1;

		float tx = ValueNoiseSmoothStep(x - x0);
		float ty = ValueNoiseSmoothStep(y - y0);
		float tz = ValueNoiseSmoothStep(z - z0);

		float c000 = ValueNoiseHash(x0, y0, z0), c100 = ValueNoiseHash(x1, y0, z0);
		float c010 = ValueNoiseHash(x0, y1, z0), c110 = ValueNoiseHash(x1, y1, z0);
		float c001 = ValueNoiseHash(x0, y0, z1), c101 = ValueNoiseHash(x1, y0, z1);
		float c011 = ValueNoiseHash(x0, y1, z1), c111 = ValueNoiseHash(x1, y1, z1);

		float x00 = c000 + tx * (c100 - c000);
		float x10 = c010 + tx * (c110 - c010);
		float x01 = c001 + tx * (c101 - c001);
		float x11 = c011 + tx * (c111 - c011);

		float y0i = x00 + ty * (x10 - x00);
		float y1i = x01 + ty * (x11 - x01);

		return y0i + tz * (y1i - y0i);
	}

	// Geometría compartida entre estalactitas (cuelgan del techo) y estalagmitas (crecen
	// del piso): mismo cono con estrechamiento no lineal, mismas estrías de ruido y misma
	// ventana de recorrido acotada al volumen del objeto. Sólo cambia desde qué extremo del
	// eje Y arranca lo ancho (attachY) y hacia dónde apunta la punta (tipY); std::abs hace
	// que la fórmula del estrechamiento no necesite ramificarse según la dirección.
	SVO::VoxelGrid CreateSpeleothem(uint32_t size, float attachFraction, float tipFraction, float baseRadiusFraction) {
		// Redondear a la potencia de 2 más cercana hacia arriba, igual que CastToVoxels
		// (el SVOBuilder asume que puede dividir el tamaño entre 2 hasta llegar a 1).
		uint32_t i = 0;
		while (static_cast<uint32_t>(std::pow(2, i)) < size) {
			i++;
		}
		size = static_cast<uint32_t>(std::pow(2, i));

		SVO::VoxelGrid vGrid(size);

		float sizeF = static_cast<float>(size);
		float center = sizeF * 0.5f;
		float attachY = attachFraction * (sizeF - 1.0f); // Extremo ancho, pegado al techo o al piso
		float tipY = tipFraction * (sizeF - 1.0f);        // La punta, sin llegar a tocar el otro extremo
		float baseRadius = sizeF * baseRadiusFraction;
		float wanderAmplitude = sizeF * 0.025f;  // Curvatura sutil del eje con la altura
		float maxRadius = baseRadius * 1.35f;    // Margen para que las estrías de ruido no se corten

		// Color "piedra caliza" con variación sutil por vóxel: a diferencia de la esfera de
		// ejemplo (color aleatorio por vóxel, se ve como estática), aquí el detalle de alta
		// resolución se nota en la forma (estrías, textura), no en ruido de color.
		auto makeMaterial = [](float shade) -> uint32_t {
			uint8_t r = static_cast<uint8_t>(std::clamp(200.0f * shade, 60.0f, 235.0f));
			uint8_t g = static_cast<uint8_t>(std::clamp(190.0f * shade, 55.0f, 225.0f));
			uint8_t b = static_cast<uint8_t>(std::clamp(175.0f * shade, 50.0f, 210.0f));
			return 0xFF000000u | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | r;
			};

		uint32_t yStart = static_cast<uint32_t>(std::min(attachY, tipY));
		uint32_t yEnd = static_cast<uint32_t>(std::max(attachY, tipY));

		for (uint32_t y = yStart; y <= yEnd; y++) {
			float fy = static_cast<float>(y);

			// t: 0 en el extremo ancho, 1 en la punta -> estrechamiento no lineal
			float t = std::clamp(std::abs(fy - attachY) / std::abs(tipY - attachY), 0.0f, 1.0f);
			float taper = std::pow(1.0f - t, 1.6f);
			if (taper <= 0.001f) continue;

			float axisX = center + wanderAmplitude * ValueNoise3D(0.0f, fy * 0.05f, 11.0f);
			float axisZ = center + wanderAmplitude * ValueNoise3D(11.0f, fy * 0.05f, 0.0f);

			// Sólo recorremos la ventana horizontal donde puede haber geometría en vez de
			// todo el plano size x size: el costo de construcción no debe escalar con el
			// mundo entero, sólo con el volumen que realmente ocupa el objeto.
			int minX = std::max(0, static_cast<int>(axisX - maxRadius));
			int maxX = std::min(static_cast<int>(size) - 1, static_cast<int>(axisX + maxRadius));
			int minZ = std::max(0, static_cast<int>(axisZ - maxRadius));
			int maxZ = std::min(static_cast<int>(size) - 1, static_cast<int>(axisZ + maxRadius));

			for (int z = minZ; z <= maxZ; z++) {
				for (int x = minX; x <= maxX; x++) {
					float dx = static_cast<float>(x) - axisX;
					float dz = static_cast<float>(z) - axisZ;
					float dist = std::sqrt(dx * dx + dz * dz);

					// Estrías/ondulaciones verticales de alta frecuencia: es este detalle el
					// que exige vóxeles pequeños para verse limpio en vez de dentado.
					float ridge = ValueNoise3D(x * 0.15f, y * 0.06f, z * 0.15f);
					float radius = std::max(baseRadius * taper * (1.0f + 0.18f * ridge), 0.0f);

					if (dist <= radius) {
						float shade = 0.75f + 0.25f * ValueNoise3D(x * 0.05f, y * 0.05f, z * 0.05f);
						vGrid.SetVoxel(static_cast<uint32_t>(x), y, static_cast<uint32_t>(z), makeMaterial(shade));
					}
				}
			}
		}

		return vGrid;
	}

	// =====================================================================================
	// Autómata de cueva
	// -------------------------------------------------------------------------------------
	// El laberinto sólo dice QUÉ celdas se comunican; tallar un prisma por pasillo da un
	// tubo recto de sección constante que se lee como un pasadizo de mina, no como una
	// cueva. Lo que sigue toma esos pasillos como semilla y les da volumen en varias
	// vueltas: cada vuelta excava un anillo más de roca (con ruido, para que el contorno
	// sea irregular) y después suaviza el resultado, así que la forma final se calcula paso
	// a paso en vez de salir de una sola fórmula cerrada.
	//
	// Dos invariantes sostienen la navegabilidad, y todo lo demás está subordinado a ellas:
	//
	//   1. PRESUPUESTO DE EXCAVACIÓN. Un vóxel de roca sólo puede volverse aire si toca aire
	//      ya excavado que aún tenga presupuesto, y el presupuesto baja de uno en uno al
	//      propagarse. Como arranca en el pasillo original, la cueva no puede crecer más de
	//      kJunctionCarveRadius vóxeles desde él. Entre los pasillos de dos celdas vecinas
	//      NO enlazadas hay 2 * (kCellsPerAxis - corridorWidth) / 2 vóxeles de roca, así que
	//      aunque ambos lados excaven al máximo siempre sobra pared: el autómata jamás abre
	//      un paso que el laberinto no había autorizado.
	//
	//   2. TUBO NAVEGABLE. Los vóxeles del pasillo original se restauran a aire al final de
	//      cada vuelta, y su mitad central (kFlagCore) queda además vetada para los
	//      espeleotemas. Da igual lo agresivo que sea el suavizado o cuántas estalagmitas
	//      caigan: siempre queda un tubo continuo de lado a lado del laberinto.
	// =====================================================================================

	// Presupuesto "sin asignar": el vóxel es roca que el autómata todavía no alcanzó.
	constexpr uint8_t kNoBudget = 0xFF;

	constexpr uint8_t kFlagNav = 1 << 0;    // Pasillo original: se restaura a aire cada vuelta
	constexpr uint8_t kFlagCore = 1 << 1;   // Mitad central del pasillo: nunca se obstruye
	constexpr uint8_t kFlagReach = 1 << 2;  // Llegó a ser candidato: el suavizado puede vaciarlo
	constexpr uint8_t kFlagSpeleo = 1 << 3; // Roca depositada por una estalactita/estalagmita
	constexpr uint8_t kFlagWall = 1 << 4;   // Tabique entre dos celdas NO enlazadas: roca intocable

	// Vueltas del autómata. Basta con que sean >= kJunctionCarveRadius para que la
	// excavación pueda gastar todo su presupuesto; las de más sólo suavizan.
	constexpr uint32_t kAutomatonRounds = 5;

	// Vóxeles que la cueva puede crecer desde el pasillo semilla. Las celdas con muchas
	// salidas (cruces) reciben el radio mayor: los cruces se abren en cámaras y los callejones
	// sin salida quedan estrechos, que es justo el contraste que hace que una cueva se lea
	// como cueva y no como un enrejado uniforme.
	//
	// Estos radios son generosos a propósito: entre dos pasillos vecinos hay 8 vóxeles de roca
	// (kCellsPerAxis - corridorWidth), así que por sí solo el presupuesto ya NO bastaría para
	// garantizar que sobre pared. Quien lo garantiza es kFlagWall, el tabique explícito que se
	// marca en la cara compartida de cada par de celdas no enlazadas. Separar las dos cosas
	// -- "hasta dónde crece" y "qué no se puede tocar" -- es lo que permite subir el radio sin
	// arriesgar el laberinto: con un radio corto las paredes nunca se rompen, pero la cueva no
	// tiene margen para deformarse y las cámaras salen cajas achaflanadas.
	constexpr uint8_t kCarveRadius = 3;
	constexpr uint8_t kJunctionCarveRadius = 4;
	constexpr uint32_t kJunctionLinks = 3;

	// Anillo de roca intocable en el borde de la región del laberinto. Por el lado bajo ese
	// borde es también el borde del mundo (el laberinto se talla desde el origen), y abrirlo
	// dejaría ver el vacío desde dentro.
	constexpr uint32_t kRegionShell = 2;

	// Espesor del tabique entre celdas no enlazadas, repartido a ambos lados de la cara que
	// comparten. Dos vóxeles bastan para que no se vea a través y para que el suavizado no lo
	// adelgace hasta romperlo.
	constexpr uint32_t kWallSheet = 2;

	// Umbral de la excavación, y cuánto sube por vuelta. Subirlo hace que cada anillo
	// sucesivo cueste más que el anterior, así la cueva se deshincha hacia afuera en vez de
	// terminar con un contorno igual de agresivo en todo su espesor.
	constexpr float kCarveThreshold = -0.02f;
	constexpr float kCarveFalloff = 0.10f;

	// Sesgos según hacia dónde está el aire que toca el vóxel de roca. Es lo que le da a la
	// cueva una silueta habitable en vez de un tubo redondo: el piso casi no se excava (queda
	// plano y caminable), el techo se levanta y las paredes se ensanchan.
	constexpr float kFloorBias = -0.25f;   // El aire está ARRIBA -> excavar hundiría el piso
	constexpr float kCeilingBias = 0.20f;  // El aire está ABAJO  -> excavar levanta el techo
	constexpr float kWallBias = 0.08f;     // El aire está al lado -> excavar ensancha

	// Reglas de mayoría del suavizado, sobre los 27 vóxeles de la vecindad 3x3x3 (él
	// incluido). Los umbrales son deliberadamente extremos: sólo quitan púas de roca rodeadas
	// de aire y rellenan grietas de un vóxel, sin tocar paredes reales (un tabique de 2
	// vóxeles tiene >= 18 vecinos de roca, muy por encima de kSmoothCarveMax).
	constexpr uint8_t kSmoothCarveMax = 8;
	constexpr uint8_t kSmoothFillMin = 20;

	// Siembra de espeleotemas: separación mínima entre piezas, hueco libre mínimo para
	// plantar una, y tope de tamaño.
	constexpr uint32_t kSpeleoStride = 5;
	constexpr uint32_t kMinClearance = 7;
	constexpr int kMaxSpeleoLength = 10;
	constexpr float kMaxSpeleoRadius = 3.0f;
	constexpr float kSpeleoWander = 0.6f;

	// Grosor de la capa de tierra sobre la roca. Sólo los vóxeles de piso a esta profundidad
	// o menos se pintan de café; debajo vuelve a ser piedra.
	constexpr uint32_t kSoilDepth = 3;

	// Escalones de matiz de la superficie. Es una decisión puramente estética -- vetas y
	// estratos anchos en vez de estática por vóxel, la misma idea que el comentario de
	// CreateSpeleothem: el detalle de una cueva se lee en la forma, no en ruido de color.
	// Medido, NO ahorra nada en el octree (825.577 nodos con color plano contra 827.673 sin
	// cuantizar, un 0.25%): la piel de la cueva es irregular vóxel a vóxel, así que el
	// SVOBuilder tiene que subdividirla por geometría pase lo que pase con el color. Lo que
	// sí importa para el tamaño del árbol es que la roca ENTERRADA conserve un material
	// constante (ver CaveMaterial); dársele color propio dispara el mismo laberinto de
	// 826k a 1.546k nodos.
	constexpr float kShadeSteps = 6.0f;

	// La región que ocupa el laberinto dentro del mundo de vóxeles, densa y aparte del
	// VoxelGrid. El mundo mide size^3 (256^3 por defecto) pero el laberinto sólo ocupa una
	// esquina de (celdas * kCellsPerAxis)^3 -- con la configuración por defecto, 60^3, un 1%
	// del mundo. Correr el autómata sobre el VoxelGrid entero sería multiplicar por ~77 el
	// trabajo para recorrer roca que nunca cambia.
	struct CaveField {
		uint32_t ex, ey, ez;
		std::vector<uint8_t> solid;  // 1 = roca, 0 = aire
		std::vector<uint8_t> budget; // Excavación que le queda al autómata; kNoBudget = sin alcanzar
		std::vector<uint8_t> flags;  // kFlagNav | kFlagCore | kFlagReach | kFlagSpeleo

		CaveField(uint32_t sx, uint32_t sy, uint32_t sz)
			: ex(sx), ey(sy), ez(sz),
			solid(static_cast<size_t>(sx) * sy * sz, 1),
			budget(static_cast<size_t>(sx) * sy * sz, kNoBudget),
			flags(static_cast<size_t>(sx) * sy * sz, 0) {
		}

		// Mismo orden que VoxelGrid: x contiguo, luego y, luego z.
		inline size_t Index(uint32_t x, uint32_t y, uint32_t z) const {
			return x + static_cast<size_t>(y) * ex + static_cast<size_t>(z) * ex * ey;
		}

		inline uint64_t AirCount() const {
			uint64_t air = 0;
			for (uint8_t v : solid) if (!v) air++;
			return air;
		}
	};

	inline float Hash01(int x, int y, int z) { return ValueNoiseHash(x, y, z) * 0.5f + 0.5f; }

	// Cuenta, para cada vóxel, cuántos de los 27 de su vecindad 3x3x3 son roca. El conteo es
	// SEPARABLE: acumular primero en X, luego en Y y luego en Z cuesta 9 sumas por vóxel en
	// vez de las 27 lecturas dispersas del bucle ingenuo. Con el laberinto por defecto da
	// igual, pero la dimensión del grid es un parámetro de la UI: a 20^3 celdas la región es
	// de 240^3 vóxeles y esa diferencia es la que separa "instantáneo" de "la ventana se
	// congela unos segundos en cada vuelta".
	//
	// Fuera de la región se cuenta roca (más allá del laberinto el mundo es piedra maciza),
	// por eso una fila fuera de rango vale 3 tras el pase en X y un plano vale 9 tras el de Y.
	void BoxCountSolid(const CaveField& f, std::vector<uint8_t>& count, std::vector<uint8_t>& scratch) {
		const size_t total = f.solid.size();
		count.assign(total, 0);
		scratch.assign(total, 0);

		for (uint32_t z = 0; z < f.ez; z++) {
			for (uint32_t y = 0; y < f.ey; y++) {
				const size_t row = f.Index(0, y, z);
				for (uint32_t x = 0; x < f.ex; x++) {
					uint32_t sum = (x == 0 ? 1u : f.solid[row + x - 1])
						+ f.solid[row + x]
						+ (x + 1 == f.ex ? 1u : f.solid[row + x + 1]);
					scratch[row + x] = static_cast<uint8_t>(sum);
				}
			}
		}

		for (uint32_t z = 0; z < f.ez; z++) {
			for (uint32_t y = 0; y < f.ey; y++) {
				const size_t row = f.Index(0, y, z);
				const size_t rowDown = row - f.ex;
				const size_t rowUp = row + f.ex;
				for (uint32_t x = 0; x < f.ex; x++) {
					uint32_t sum = (y == 0 ? 3u : scratch[rowDown + x])
						+ scratch[row + x]
						+ (y + 1 == f.ey ? 3u : scratch[rowUp + x]);
					count[row + x] = static_cast<uint8_t>(sum);
				}
			}
		}

		const size_t plane = static_cast<size_t>(f.ex) * f.ey;
		for (uint32_t z = 0; z < f.ez; z++) {
			for (uint32_t y = 0; y < f.ey; y++) {
				const size_t row = f.Index(0, y, z);
				for (uint32_t x = 0; x < f.ex; x++) {
					uint32_t sum = (z == 0 ? 9u : count[row + x - plane])
						+ count[row + x]
						+ (z + 1 == f.ez ? 9u : count[row + x + plane]);
					scratch[row + x] = static_cast<uint8_t>(sum);
				}
			}
		}

		count.swap(scratch);
	}

	// Una vuelta de excavación: cada vóxel de roca que toca aire con presupuesto consulta el
	// campo de ruido para decidir si se abre. Se lee de una foto del estado anterior para que
	// el resultado no dependa del orden del recorrido (si no, la cueva crecería más hacia +X
	// que hacia -X dentro de una misma vuelta).
	uint32_t ErosionRound(CaveField& f, uint32_t round) {
		const std::vector<uint8_t> prevSolid = f.solid;
		const std::vector<uint8_t> prevBudget = f.budget;
		const float threshold = kCarveThreshold + kCarveFalloff * static_cast<float>(round);
		uint32_t carved = 0;

		for (uint32_t z = kRegionShell; z + kRegionShell < f.ez; z++) {
			for (uint32_t y = kRegionShell; y + kRegionShell < f.ey; y++) {
				for (uint32_t x = kRegionShell; x + kRegionShell < f.ex; x++) {
					const size_t i = f.Index(x, y, z);
					if (!prevSolid[i]) continue;
					if (f.flags[i] & kFlagWall) continue; // Invariante 1: el tabique no se toca

					// De los 6 vecinos de cara, quedarse con el que más presupuesto ofrece y
					// con el sesgo direccional más favorable.
					int bestBudget = -1;
					float bias = 0.0f;
					bool anyNeighbor = false;

					auto consider = [&](size_t j, float dirBias) {
						if (prevSolid[j]) return;
						const uint8_t b = prevBudget[j];
						if (b == kNoBudget || b == 0) return;
						if (static_cast<int>(b) > bestBudget) bestBudget = static_cast<int>(b);
						bias = anyNeighbor ? std::max(bias, dirBias) : dirBias;
						anyNeighbor = true;
						};

					consider(f.Index(x, y + 1, z), kFloorBias);   // Aire arriba -> este vóxel es piso
					consider(f.Index(x, y - 1, z), kCeilingBias); // Aire abajo  -> este vóxel es techo
					consider(f.Index(x + 1, y, z), kWallBias);
					consider(f.Index(x - 1, y, z), kWallBias);
					consider(f.Index(x, y, z + 1), kWallBias);
					consider(f.Index(x, y, z - 1), kWallBias);

					if (!anyNeighbor) continue;

					// Candidato alcanzado: aunque no se excave ahora, el suavizado ya podrá
					// limpiarlo si queda como una púa suelta.
					f.flags[i] |= kFlagReach;

					// Tres octavas de un campo de ruido FIJO en el espacio, que no cambia
					// entre vueltas. Esto último es lo que decide si la cueva sale orgánica o
					// no: con un ruido que se re-sortea cada vuelta, un vóxel de pared tira
					// cuatro dados independientes y casi cualquiera acaba abriéndose, así que
					// la excavación degenera en dilatar el pasillo un grosor uniforme y las
					// cámaras salen cajas de esquinas achaflanadas. Con el campo fijo, el
					// contorno se pega a una isosuperficie del ruido: la octava larga decide
					// qué tramos se ensanchan en cámara y cuáles quedan angostos, y la corta
					// pone la rugosidad vóxel a vóxel. Las vueltas siguen haciendo falta
					// porque un vóxel sólo puede abrirse cuando el de adentro ya se abrió: el
					// ruido dice HASTA DÓNDE, las vueltas hacen que llegue anillo por anillo.
					const float score = 0.45f * ValueNoise3D(x * 0.045f, y * 0.050f, z * 0.045f)
						+ 0.33f * ValueNoise3D(x * 0.120f, y * 0.130f, z * 0.120f)
						+ 0.22f * ValueNoise3D(x * 0.300f, y * 0.320f, z * 0.300f)
						+ bias;
					if (score <= threshold) continue;

					f.solid[i] = 0;
					f.budget[i] = static_cast<uint8_t>(bestBudget - 1);
					carved++;
				}
			}
		}

		return carved;
	}

	// Una vuelta de suavizado por regla de mayoría, más la restauración del tubo navegable.
	// Vaciar sólo se permite donde el autómata llegó (kFlagReach): así el suavizado nunca
	// perfora una pared que la excavación no tenía permitido tocar.
	uint32_t SmoothingRound(CaveField& f, std::vector<uint8_t>& count, std::vector<uint8_t>& scratch) {
		BoxCountSolid(f, count, scratch);
		uint32_t changed = 0;

		for (uint32_t z = kRegionShell; z + kRegionShell < f.ez; z++) {
			for (uint32_t y = kRegionShell; y + kRegionShell < f.ey; y++) {
				for (uint32_t x = kRegionShell; x + kRegionShell < f.ex; x++) {
					const size_t i = f.Index(x, y, z);
					const uint8_t neighbors = count[i];

					if (f.solid[i]) {
						if ((f.flags[i] & kFlagReach) && !(f.flags[i] & kFlagWall) && neighbors <= kSmoothCarveMax) {
							f.solid[i] = 0;
							changed++;
						}
					}
					else if (neighbors >= kSmoothFillMin) {
						f.solid[i] = 1;
						changed++;
					}
				}
			}
		}

		// Invariante 2: el pasillo original vuelve a ser aire pase lo que pase.
		for (size_t i = 0; i < f.solid.size(); i++) {
			if (f.flags[i] & kFlagNav) f.solid[i] = 0;
		}

		return changed;
	}

	struct SpeleothemParams {
		int dir;              // -1 cuelga hacia abajo (estalactita), +1 crece hacia arriba (estalagmita)
		float lengthFraction; // Fracción del hueco libre que ocupa la pieza
		float radiusFactor;   // Radio de la base en función del largo
		float density;        // Probabilidad de sembrar en un anclaje válido
		int salt;             // Desplaza el hash para que las dos pasadas no coincidan
	};

	// Siembra espeleotemas sobre los techos y pisos que dejó el autómata. A diferencia de
	// CreateStalactite/CreateStalagmite (que dedican un mundo entero a una sola pieza de alta
	// resolución), aquí interesan muchas piezas pequeñas repartidas por la cueva, así que se
	// reutiliza la misma geometría -- cono con estrechamiento no lineal y estrías de ruido --
	// a la escala que permite el hueco.
	//
	// El largo se mide siempre como una fracción del hueco libre que hay delante del anclaje,
	// nunca en vóxeles absolutos: por construcción la pieza no puede sellar un paso. Y las
	// estalactitas se depositan ANTES que las estalagmitas a propósito, porque al medir su
	// hueco la estalagmita ya "ve" la estalactita que le cuelga encima y se acorta sola.
	uint32_t GrowSpeleothems(CaveField& f, const SpeleothemParams& p) {
		const int shell = static_cast<int>(kRegionShell);
		const int height = static_cast<int>(f.ey);
		uint32_t placed = 0;

		for (uint32_t gz = kRegionShell; gz + kRegionShell < f.ez; gz += kSpeleoStride) {
			for (uint32_t gx = kRegionShell; gx + kRegionShell < f.ex; gx += kSpeleoStride) {

				// Sin este desplazamiento pseudoaleatorio dentro de la celda de siembra las
				// piezas quedarían alineadas en una retícula perfectamente visible.
				const uint32_t x = gx + static_cast<uint32_t>(Hash01(gx, p.salt, gz) * kSpeleoStride);
				const uint32_t z = gz + static_cast<uint32_t>(Hash01(gz, p.salt + 91, gx) * kSpeleoStride);
				if (x + kRegionShell >= f.ex || z + kRegionShell >= f.ez) continue;

				for (int y = shell; y + shell < height; y++) {
					if (!f.solid[f.Index(x, static_cast<uint32_t>(y), z)]) continue;

					// Anclaje: roca con aire justo del lado hacia el que crecería la pieza.
					const int baseY = y + p.dir;
					if (baseY < shell || baseY + shell >= height) continue;
					if (f.solid[f.Index(x, static_cast<uint32_t>(baseY), z)]) continue;

					int clearance = 0;
					for (int cy = baseY; cy >= shell && cy + shell < height; cy += p.dir) {
						if (f.solid[f.Index(x, static_cast<uint32_t>(cy), z)]) break;
						clearance++;
					}
					if (clearance < static_cast<int>(kMinClearance)) continue;

					if (Hash01(static_cast<int>(x), y * 7 + p.salt, static_cast<int>(z)) > p.density) {
						y += clearance; // No reintentar dentro del mismo hueco
						continue;
					}

					const int length = std::clamp(static_cast<int>(std::lround(clearance * p.lengthFraction)), 2, kMaxSpeleoLength);
					const float baseRadius = std::clamp(length * p.radiusFactor, 1.1f, kMaxSpeleoRadius);
					const float maxRadius = baseRadius * 1.4f;

					for (int step = 0; step < length; step++) {
						const int vy = baseY + p.dir * step;
						if (vy < shell || vy + shell >= height) break;

						// t: 0 en el extremo pegado a la roca, 1 en la punta.
						const float t = (length > 1) ? static_cast<float>(step) / static_cast<float>(length - 1) : 1.0f;
						const float radiusAtStep = baseRadius * std::pow(1.0f - t, 1.6f);
						if (radiusAtStep < 0.35f) continue;

						const float axisX = static_cast<float>(x) + kSpeleoWander * ValueNoise3D(0.0f, vy * 0.06f, static_cast<float>(z));
						const float axisZ = static_cast<float>(z) + kSpeleoWander * ValueNoise3D(static_cast<float>(x), vy * 0.06f, 0.0f);

						const int minX = std::max(shell, static_cast<int>(std::floor(axisX - maxRadius)));
						const int maxX = std::min(static_cast<int>(f.ex) - shell - 1, static_cast<int>(std::ceil(axisX + maxRadius)));
						const int minZ = std::max(shell, static_cast<int>(std::floor(axisZ - maxRadius)));
						const int maxZ = std::min(static_cast<int>(f.ez) - shell - 1, static_cast<int>(std::ceil(axisZ + maxRadius)));

						for (int zz = minZ; zz <= maxZ; zz++) {
							for (int xx = minX; xx <= maxX; xx++) {
								const float dx = static_cast<float>(xx) - axisX;
								const float dz = static_cast<float>(zz) - axisZ;
								const float dist = std::sqrt(dx * dx + dz * dz);

								const float ridge = ValueNoise3D(xx * 0.28f, vy * 0.14f, zz * 0.28f);
								if (dist > radiusAtStep * (1.0f + 0.22f * ridge)) continue;

								const size_t j = f.Index(static_cast<uint32_t>(xx), static_cast<uint32_t>(vy), static_cast<uint32_t>(zz));
								if (f.solid[j]) continue;                 // No pisar roca que ya existe
								if (f.flags[j] & kFlagCore) continue;     // Invariante 2: el tubo central no se obstruye

								f.solid[j] = 1;
								f.flags[j] |= kFlagSpeleo;
							}
						}
					}

					placed++;
					y += clearance; // Una sola pieza por hueco en esta columna
				}
			}
		}

		return placed;
	}

	inline uint32_t PackColor(float r, float g, float b) {
		const uint8_t ri = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
		const uint8_t gi = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
		const uint8_t bi = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
		return 0xFF000000u | (static_cast<uint32_t>(bi) << 16) | (static_cast<uint32_t>(gi) << 8) | ri;
	}

	// Color de un vóxel sólido según a qué está expuesto. Sólo la piel de la cueva recibe
	// color propio: la roca enterrada conserva 'buried', un material CONSTANTE, para que el
	// SVOBuilder pueda fusionar bloques enteros en un único nodo (ver PASO 2 de
	// SVOBuilder::BuildRecursive) en vez de subdividir hasta la hoja en volumen que nadie
	// llega a ver nunca. No es un detalle menor: darle color propio a la roca enterrada casi
	// duplica el octree (826k -> 1.546k nodos en un laberinto de 10^3 celdas).
	uint32_t CaveMaterial(const CaveField& f, uint32_t x, uint32_t y, uint32_t z, uint32_t buried) {
		const size_t i = f.Index(x, y, z);

		const bool airAbove = (y + 1 < f.ey) && !f.solid[f.Index(x, y + 1, z)];
		const bool airBelow = (y > 0) && !f.solid[f.Index(x, y - 1, z)];
		const bool airSide = ((x + 1 < f.ex) && !f.solid[f.Index(x + 1, y, z)])
			|| ((x > 0) && !f.solid[f.Index(x - 1, y, z)])
			|| ((z + 1 < f.ez) && !f.solid[f.Index(x, y, z + 1)])
			|| ((z > 0) && !f.solid[f.Index(x, y, z - 1)]);

		if (!airAbove && !airBelow && !airSide && !(f.flags[i] & kFlagSpeleo)) return buried;

		// Ruido de matiz cuantizado: ver kShadeSteps.
		const float noise01 = ValueNoise3D(x * 0.07f, y * 0.07f, z * 0.07f) * 0.5f + 0.5f;
		const float shade = std::clamp(std::floor(noise01 * kShadeSteps), 0.0f, kShadeSteps - 1.0f) / (kShadeSteps - 1.0f);

		// Los espeleotemas se resuelven primero porque su exposición engaña: una estalagmita
		// tiene aire encima (parecería piso) y una estalactita lo tiene debajo. Son calcita,
		// no tierra ni pared, y usan la misma paleta que CreateSpeleothem.
		if (f.flags[i] & kFlagSpeleo) {
			const float k = 0.78f + 0.22f * shade;
			return PackColor(200.0f * k, 190.0f * k, 175.0f * k);
		}

		// Piso: los kSoilDepth vóxeles bajo la superficie son tierra, en cafés y marrones que
		// se oscurecen con la profundidad. Se exige que NO haya aire debajo para que una losa
		// delgada (que por abajo es el techo de otra galería) no se pinte de tierra.
		if (!airBelow) {
			int soilDepth = -1;
			for (uint32_t d = 1; d <= kSoilDepth && y + d < f.ey; d++) {
				if (!f.solid[f.Index(x, y + d, z)]) { soilDepth = static_cast<int>(d) - 1; break; }
			}
			if (soilDepth >= 0) {
				const float dim = 1.0f - 0.16f * static_cast<float>(soilDepth);
				return PackColor((86.0f + shade * 70.0f) * dim,   // 86 -> 156
					(58.0f + shade * 54.0f) * dim,               // 58 -> 112
					(34.0f + shade * 34.0f) * dim);              // 34 -> 68
			}
		}

		// Pared y techo: piedra gris, con el techo algo más oscuro para que la profundidad se
		// lea aun sin tocar la iluminación.
		const float dim = airBelow ? 0.78f : 1.0f;
		return PackColor((92.0f + shade * 76.0f) * dim,   // 92 -> 168
			(94.0f + shade * 76.0f) * dim,               // 94 -> 170
			(100.0f + shade * 77.0f) * dim);             // 100 -> 177
	}

}

namespace SVO {

	std::string VoxelGrid::ToString() const {
		std::string result;
		uint64_t blocksCount = 0;

		for (uint32_t z = 0; z < m_Size; z++) {
			//result += "Layer " + std::to_string(z) + ":\n";
			for (uint32_t y = 0; y < m_Size; y++) {
				for (uint32_t x = 0; x < m_Size; x++) {
					if (GetVoxel(x, y, z)) {
						//			result += "#";
						blocksCount++;
					}
					else {
						//			result += ".";
					}
				}
				//	result += "\n";
			}
			//result += "\n";
		}

		// m_Data siempre tiene m_Size^3 elementos (denso, incluye aire), así que la memoria
		// real ocupada depende del tamaño del grid, no de cuántos bloques son sólidos.
		uint64_t totalVoxels = static_cast<uint64_t>(m_Data.size());
		uint64_t memoryBytes = totalVoxels * sizeof(uint32_t);

		result += "Total voxel grid size: " + std::to_string(totalVoxels) + "\n";
		result += "Total non air blocks: " + std::to_string(blocksCount) + "\n";
		result += "Memory usage: " + FormatBytesHumanReadable(memoryBytes) + "\n";

		return result;
	}

	VoxelGrid VoxelGrid::CastToVoxels(Maze::Grid3D* laberinto, int gridSize, const ProgressFn& progress) {
		uint32_t newCellsPerAxis = kCellsPerAxis;

		// El laberinto debe caber dentro del grid: cada celda ocupa un bloque de
		// newCellsPerAxis^3, así que el grid necesita al menos esa extensión.
		uint32_t requiredExtent = std::max({ laberinto->GetColumns(), laberinto->GetLevels(), laberinto->GetRows() }) * newCellsPerAxis;
		gridSize = std::max(gridSize, static_cast<int>(requiredExtent));

		uint32_t i = 0;
		while (static_cast<uint32_t>(std::pow(2, i)) < static_cast<uint32_t>(gridSize)) {
			i++;
		}

		gridSize = static_cast<uint32_t>(std::pow(2, i));

		VoxelGrid vGrid(gridSize);

		// Material constante de "piedra sólida" (RGBA empaquetado, alpha != 0 para que el
		// ray marcher lo trate como opaco). Debe ser constante para que el SVOBuilder pueda
		// fusionar bloques sólidos en vez de tener que subdividir hasta el voxel individual.
		constexpr uint32_t kSolidStoneMaterial = 0xFF808080;

		// 1. Llenar TODO el mundo de piedra sólida
		vGrid.Fill(kSolidStoneMaterial);

		// Ancho del pasillo/cámara en vóxeles de índice (misma proporción que la versión
		// anterior de 1 vóxel de ancho en un bloque de 3, pero escalado a la resolución
		// actual) y desplazamiento para centrarlo dentro del bloque de la celda.
		uint32_t corridorWidth = newCellsPerAxis / 3;
		uint32_t chamberOffset = (newCellsPerAxis - corridorWidth) / 2;

		// Mitad central del pasillo. Es el tubo que ni el suavizado ni los espeleotemas
		// pueden obstruir: la garantía dura de que el laberinto sigue siendo recorrible de
		// punta a punta por mucho que el autómata deforme todo lo demás (invariante 2).
		uint32_t coreWidth = std::max(2u, corridorWidth / 2);
		uint32_t coreOffset = (newCellsPerAxis - coreWidth) / 2;

		// 2. Sembrar la región del laberinto: se talla sobre un campo denso aparte, no sobre
		//    el VoxelGrid, porque el autómata necesita leer y reescribir el mismo vóxel
		//    muchas veces y además llevarle la cuenta de presupuesto y banderas. El VoxelGrid
		//    sólo se toca una vez, al final, para pintar el resultado.
		CaveField field(laberinto->GetColumns() * newCellsPerAxis,
			laberinto->GetLevels() * newCellsPerAxis,
			laberinto->GetRows() * newCellsPerAxis);

		auto carveBox = [&](uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, uint32_t z0, uint32_t z1, uint8_t budget, uint8_t flag) {
			for (uint32_t z = z0; z < z1; z++)
				for (uint32_t y = y0; y < y1; y++)
					for (uint32_t x = x0; x < x1; x++) {
						size_t index = field.Index(x, y, z);
						field.solid[index] = 0;
						field.flags[index] |= flag;
						// El presupuesto arranca en el pasillo semilla y se propaga hacia
						// afuera restando 1 por anillo (ver ErosionRound): es lo que acota
						// cuánto puede crecer la cueva (invariante 1).
						if (budget > field.budget[index] || field.budget[index] == kNoBudget) field.budget[index] = budget;
					}
			};

		laberinto->EachCell([&](Maze::Cell3D* cell) {

			// Coordenada base del bloque de la celda en el mundo de vóxeles
			uint32_t bx = cell->GetColumn() * newCellsPerAxis;
			uint32_t by = cell->GetLevel() * newCellsPerAxis;
			uint32_t bz = cell->GetRow() * newCellsPerAxis;

			// Los cruces se abren en cámaras y los callejones sin salida quedan estrechos.
			uint8_t budget = cell->GetLinks().size() >= kJunctionLinks ? kJunctionCarveRadius : kCarveRadius;

			// Cada tramo se talla dos veces: el pasillo completo (kFlagNav, se restaura a
			// aire tras cada vuelta) y su mitad central (kFlagCore, además vetada para los
			// espeleotemas).
			auto carveCorridor = [&](uint32_t off, uint32_t width, uint8_t flag) {
				// Cámara central de la celda
				carveBox(bx + off, bx + off + width, by + off, by + off + width, bz + off, bz + off + width, budget, flag);

				// Pasillos hacia los vecinos enlazados
				if (cell->IsLinked(cell->GetEast()))
					carveBox(bx + off + width, bx + newCellsPerAxis, by + off, by + off + width, bz + off, bz + off + width, budget, flag);
				if (cell->IsLinked(cell->GetWest()))
					carveBox(bx, bx + off, by + off, by + off + width, bz + off, bz + off + width, budget, flag);
				if (cell->IsLinked(cell->GetSouth())) // Sur suele ser +Z
					carveBox(bx + off, bx + off + width, by + off, by + off + width, bz + off + width, bz + newCellsPerAxis, budget, flag);
				if (cell->IsLinked(cell->GetNorth()))
					carveBox(bx + off, bx + off + width, by + off, by + off + width, bz, bz + off, budget, flag);
				if (cell->IsLinked(cell->GetUp()))
					carveBox(bx + off, bx + off + width, by + off + width, by + newCellsPerAxis, bz + off, bz + off + width, budget, flag);
				if (cell->IsLinked(cell->GetDown()))
					carveBox(bx + off, bx + off + width, by, by + off, bz + off, bz + off + width, budget, flag);
				};

			carveCorridor(chamberOffset, corridorWidth, kFlagNav);
			carveCorridor(coreOffset, coreWidth, kFlagNav | kFlagCore);
			});

		// 2b. Marcar el tabique entre celdas NO enlazadas: una lámina de kWallSheet vóxeles de
		//     espesor sobre toda la cara compartida, que el autómata tiene prohibido excavar.
		//     Es la garantía DURA de que la topología del laberinto sobrevive (invariante 1):
		//     no queda ningún camino de aire que cruce una cara no enlazada, así que el
		//     autómata no puede inventar atajos por mucho que ensanche las galerías.
		//     Va en un segundo recorrido y no dentro del anterior porque necesita que TODOS
		//     los pasillos estén ya tallados para poder saltarse los que cruzan una cara.
		auto sealFace = [&](uint32_t bx, uint32_t by, uint32_t bz, int axis) {
			for (uint32_t d = 0; d < kWallSheet; d++) {
				// La lámina arranca en el último vóxel del bloque de esta celda y sigue hacia
				// el bloque vecino, así que queda montada sobre el plano que los separa.
				uint32_t offset = newCellsPerAxis - 1 + d;

				for (uint32_t a = 0; a < newCellsPerAxis; a++) {
					for (uint32_t b = 0; b < newCellsPerAxis; b++) {
						uint32_t x = bx, y = by, z = bz;
						if (axis == 0) { x += offset; y += a; z += b; }
						else if (axis == 1) { x += a; y += offset; z += b; }
						else { x += a; y += b; z += offset; }

						if (x >= field.ex || y >= field.ey || z >= field.ez) continue;
						size_t index = field.Index(x, y, z);
						if (field.flags[index] & kFlagNav) continue; // Nunca sellar un pasillo real
						field.flags[index] |= kFlagWall;
					}
				}
			}
			};

		laberinto->EachCell([&](Maze::Cell3D* cell) {
			uint32_t bx = cell->GetColumn() * newCellsPerAxis;
			uint32_t by = cell->GetLevel() * newCellsPerAxis;
			uint32_t bz = cell->GetRow() * newCellsPerAxis;

			// Sólo las caras "hacia adelante": la cara opuesta la sella la celda vecina.
			if (cell->GetEast() && !cell->IsLinked(cell->GetEast())) sealFace(bx, by, bz, 0);
			if (cell->GetUp() && !cell->IsLinked(cell->GetUp())) sealFace(bx, by, bz, 1);
			if (cell->GetSouth() && !cell->IsLinked(cell->GetSouth())) sealFace(bx, by, bz, 2);
			});

		// 3. Correrle las vueltas al autómata. Cada vuelta = excavar un anillo más de roca
		//    con ruido + suavizar el contorno resultante + restaurar el tubo navegable.
		// Reparto aproximado del tiempo medido: el autómata es casi todo el costo.
		if (!ReportProgress(progress, 0.05f)) return VoxelGrid(0);

		std::vector<uint8_t> neighborCount, scratch;
		for (uint32_t round = 0; round < kAutomatonRounds; round++) {
			uint32_t carved = ErosionRound(field, round);
			uint32_t smoothed = SmoothingRound(field, neighborCount, scratch);
			if (!ReportProgress(progress, 0.05f + 0.75f * static_cast<float>(round + 1) / kAutomatonRounds)) return VoxelGrid(0);
			LR_CORE_TRACE("Cueva: vuelta {0}/{1} -> {2} voxeles excavados, {3} corregidos por suavizado",
				round + 1, kAutomatonRounds, carved, smoothed);
		}

		// 4. Espeleotemas sobre los techos y pisos que quedaron. Las estalactitas primero: la
		//    estalagmita mide su hueco después y se acorta sola bajo la que le cuelga encima.
		uint32_t stalactites = GrowSpeleothems(field, { -1, 0.45f, 0.30f, 0.55f, 0 });
		uint32_t stalagmites = GrowSpeleothems(field, { +1, 0.38f, 0.34f, 0.45f, 613 });
		if (!ReportProgress(progress, 0.9f)) return VoxelGrid(0);

		// 5. Volcar el campo al VoxelGrid pintando la piel de la cueva: tierra en cafés para
		//    el piso, piedra gris para paredes y techo, calcita para los espeleotemas. La roca
		//    enterrada conserva kSolidStoneMaterial y se funde con el resto del mundo, que ya
		//    está lleno de ese mismo material.
		for (uint32_t z = 0; z < field.ez; z++)
			for (uint32_t y = 0; y < field.ey; y++)
				for (uint32_t x = 0; x < field.ex; x++) {
					uint32_t material = field.solid[field.Index(x, y, z)]
						? CaveMaterial(field, x, y, z, kSolidStoneMaterial)
						: 0;
					vGrid.SetVoxel(x, y, z, material);
				}

		LR_CORE_INFO("Cueva generada: region {0}x{1}x{2}, {3} voxeles de aire, {4} estalactitas, {5} estalagmitas",
			field.ex, field.ey, field.ez, field.AirCount(), stalactites, stalagmites);
		ReportProgress(progress, 1.0f);

		return vGrid;
	}

	VoxelGrid VoxelGrid::CreateStalactite(uint32_t size) {
		// Cuelga del techo (extremo ancho en el borde superior) y apunta casi hasta el piso.
		return CreateSpeleothem(size, /*attachFraction*/ 1.0f, /*tipFraction*/ 0.08f, /*baseRadiusFraction*/ 0.22f);
	}

	VoxelGrid VoxelGrid::CreateStalagmite(uint32_t size) {
		// Crece del piso (extremo ancho en el borde inferior) y apunta casi hasta el techo.
		// Base un poco más ancha que la estalactita: las estalagmitas reales suelen ser más
		// rechonchas porque se forman por acumulación de goteo en vez de erosión colgante.
		return CreateSpeleothem(size, /*attachFraction*/ 0.0f, /*tipFraction*/ 0.92f, /*baseRadiusFraction*/ 0.26f);
	}

}
