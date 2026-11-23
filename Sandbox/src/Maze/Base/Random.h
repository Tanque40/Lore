#pragma once

#include <cstdint>
#include <random>

namespace Maze {

	// Generador de números aleatorios con semilla explícita para los algoritmos de laberinto.
	// Reemplaza a std::rand(): con la misma semilla se obtiene exactamente el mismo
	// laberinto en cualquier plataforma (mundos reproducibles y pruebas deterministas), y
	// Range() usa una distribución uniforme en vez de "rand() % n", que tiene sesgo de módulo.
	class Random {
	private:
		std::mt19937_64 m_Engine;
		uint64_t m_Seed;

	public:
		explicit Random(uint64_t seed) : m_Engine(seed), m_Seed(seed) {}

		inline uint64_t GetSeed() const { return m_Seed; }

		// Entero uniforme en [0, n). n debe ser > 0.
		inline uint32_t Range(uint32_t n) {
			std::uniform_int_distribution<uint32_t> dist(0, n - 1);
			return dist(m_Engine);
		}

		// true con probabilidad 1/2.
		inline bool Coin() { return Range(2) == 0; }

		// Generador compartido para el código que todavía no pasa un Random explícito.
		// Se siembra una vez con std::random_device; para reproducir un laberinto, pasa tu
		// propio Random con una semilla conocida a los algoritmos.
		static Random& Global() {
			static Random s_Global(std::random_device{}());
			return s_Global;
		}
	};

}
