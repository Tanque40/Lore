#include "sndbxpch.h"

#include <iomanip>
#include <sstream>

#include "SVO/VoxelGrid.h"

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

	VoxelGrid VoxelGrid::CastToVoxels(Maze::Grid3D* laberinto, int gridSize) {
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
		for (uint32_t x = 0; x < gridSize; x++) {
			for (uint32_t y = 0; y < gridSize; y++) {
				for (uint32_t z = 0; z < gridSize; z++) {
					vGrid.SetVoxel(x, y, z, kSolidStoneMaterial);
				}
			}
		}

		// Ancho del pasillo/cámara en vóxeles de índice (misma proporción que la versión
		// anterior de 1 vóxel de ancho en un bloque de 3, pero escalado a la resolución
		// actual) y desplazamiento para centrarlo dentro del bloque de la celda.
		uint32_t corridorWidth = newCellsPerAxis / 3;
		uint32_t chamberOffset = (newCellsPerAxis - corridorWidth) / 2;

		auto carveBox = [&](uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, uint32_t z0, uint32_t z1) {
			for (uint32_t x = x0; x < x1; x++)
				for (uint32_t y = y0; y < y1; y++)
					for (uint32_t z = z0; z < z1; z++)
						vGrid.SetVoxel(x, y, z, 0);
			};

		// 2. Esculpir los pasillos iterando sobre el Grid3D
		laberinto->EachCell([&](Maze::Cell3D* cell) {

			// Coordenada base del bloque de la celda en el mundo de vóxeles
			uint32_t bx = cell->GetColumn() * newCellsPerAxis;
			uint32_t by = cell->GetLevel() * newCellsPerAxis;
			uint32_t bz = cell->GetRow() * newCellsPerAxis;

			// Cámara central de la celda
			carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
				by + chamberOffset, by + chamberOffset + corridorWidth,
				bz + chamberOffset, bz + chamberOffset + corridorWidth);

			// Pasillos hacia los vecinos enlazados
			if (cell->IsLinked(cell->GetEast())) {
				carveBox(bx + chamberOffset + corridorWidth, bx + newCellsPerAxis,
					by + chamberOffset, by + chamberOffset + corridorWidth,
					bz + chamberOffset, bz + chamberOffset + corridorWidth);
			}
			if (cell->IsLinked(cell->GetWest())) {
				carveBox(bx, bx + chamberOffset,
					by + chamberOffset, by + chamberOffset + corridorWidth,
					bz + chamberOffset, bz + chamberOffset + corridorWidth);
			}
			if (cell->IsLinked(cell->GetSouth())) { // Sur suele ser +Z
				carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
					by + chamberOffset, by + chamberOffset + corridorWidth,
					bz + chamberOffset + corridorWidth, bz + newCellsPerAxis);
			}
			if (cell->IsLinked(cell->GetNorth())) {
				carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
					by + chamberOffset, by + chamberOffset + corridorWidth,
					bz, bz + chamberOffset);
			}
			if (cell->IsLinked(cell->GetUp())) {
				carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
					by + chamberOffset + corridorWidth, by + newCellsPerAxis,
					bz + chamberOffset, bz + chamberOffset + corridorWidth);
			}
			if (cell->IsLinked(cell->GetDown())) {
				carveBox(bx + chamberOffset, bx + chamberOffset + corridorWidth,
					by, by + chamberOffset,
					bz + chamberOffset, bz + chamberOffset + corridorWidth);
			}

			});

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
