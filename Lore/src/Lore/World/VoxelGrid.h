#pragma once

#include "Lore/Log.h"

#include "Lore/ProcGen/Maze/Base/Grid3D.h"

namespace SVO {

	// Callback de progreso para trabajos largos que corren en un hilo del JobSystem.
	// Recibe la fracción completada (0-1) y devuelve false para pedir que se cancele
	// (p. ej. porque la app se está cerrando). Puede ser vacío.
	using ProgressFn = std::function<bool(float)>;

	inline bool ReportProgress(const ProgressFn& progress, float fraction) {
		return !progress || progress(fraction);
	}

	// Un grid volumétrico temporal para la CPU
	class VoxelGrid {
	private:
		std::vector<uint32_t> m_Data;
		uint32_t m_Size; // Debe ser potencia de 2 (ej. 64, 128)

	public:
		// Cuántos vóxeles de índice ocupa cada celda del laberinto por eje. Junto con
		// el uniform u_VoxelScale del renderer (mundo = índice * voxelScale) define el
		// tamaño físico real de una celda; subir este número sin tocar voxelScale sólo
		// añade resolución (vóxeles más finos) sin cambiar el tamaño del laberinto.
		static constexpr uint32_t kCellsPerAxis = 12;


		explicit VoxelGrid(uint32_t size = 0) : m_Size(size) {
			// Inicializar todo como aire (0)
			m_Data.resize(static_cast<size_t>(size) * size * size, 0);
		}

		// Rellena el mundo entero con un único material. std::fill recorre m_Data de forma
		// lineal; el triple bucle con SetVoxel que hacía esto antes escribía con paso
		// m_Size^2 en el bucle interno, o sea un fallo de caché por vóxel en los 16.7M de un
		// mundo de 256^3.
		inline void Fill(uint32_t material) {
			std::fill(m_Data.begin(), m_Data.end(), material);
		}

		inline void Clear() {
			m_Data.clear();
			m_Size = 0;
		}

		inline void SetVoxel(uint32_t x, uint32_t y, uint32_t z, uint32_t material) {
			LR_ASSERT(x < m_Size && y < m_Size && z < m_Size, "VoxelGrid::SetVoxel out of bounds");
			m_Data[x + y * m_Size + z * m_Size * m_Size] = material;
		}

		inline uint32_t GetVoxel(uint32_t x, uint32_t y, uint32_t z) const {
			LR_ASSERT(x < m_Size && y < m_Size && z < m_Size, "VoxelGrid::GetVoxel out of bounds");
			return m_Data[x + y * m_Size + z * m_Size * m_Size];
		}

		inline uint32_t GetSize() const { return m_Size; }
		// Índice lineal x + y*size + z*size^2 (mismo orden que SetVoxel/GetVoxel).
		inline const std::vector<uint32_t>& GetData() const { return m_Data; }

		std::string ToString() const;

		// Si progress pide cancelar, devuelve un grid vacío (GetSize() == 0).
		static VoxelGrid CastToVoxels(Maze::Grid3D* laberinto, int gridSize, const ProgressFn& progress = {});

		// Genera un mundo dedicado por completo a una única estalactita de alta resolución
		// con estrías/ondulaciones procedurales (ruido Perlin). A diferencia de CastToVoxels
		// (que reparte el presupuesto size^3 sobre un laberinto grande), aquí TODO el índice
		// se concentra en el volumen que realmente ocupa el objeto: el mismo costo de memoria
		// (size^3, ver comentario en el constructor) produce vóxeles mucho más finos sobre la
		// única pieza de geometría que importa. size se redondea a la potencia de 2 más
		// cercana hacia arriba, igual que CastToVoxels.
		static VoxelGrid CreateStalactite(uint32_t size);

		// Misma idea que CreateStalactite pero invertida: crece del piso hacia arriba en vez
		// de colgar del techo hacia abajo (y con una base algo más ancha, como una
		// estalagmita real).
		static VoxelGrid CreateStalagmite(uint32_t size);
	};
}
