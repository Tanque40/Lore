#pragma once

#include <atomic>
#include <mutex>
#include <string>

namespace Lore {

	// Canal entre un Scene::OnLoad que corre en un hilo del JobSystem y el hilo principal
	// (que dibuja la pantalla de carga). Todo es seguro para llamar desde cualquier hilo.
	class LoadContext {
	private:
		std::atomic<float> m_Progress{ 0.0f };
		std::atomic<bool> m_Cancelled{ false };
		mutable std::mutex m_StageMutex;
		std::string m_Stage;

	public:
		// Fracción global 0-1. Sólo avanza: un valor menor que el actual se ignora, así la
		// barra nunca retrocede aunque dos fases reporten con escalas distintas.
		void SetProgress(float progress) {
			progress = progress < 0.0f ? 0.0f : (progress > 1.0f ? 1.0f : progress);
			float current = m_Progress.load(std::memory_order_relaxed);
			while (progress > current && !m_Progress.compare_exchange_weak(current, progress, std::memory_order_relaxed)) {}
		}
		float GetProgress() const { return m_Progress.load(std::memory_order_relaxed); }

		// Texto de la fase actual ("Generando laberinto", "Construyendo SVO", ...).
		void SetStage(std::string stage) {
			std::lock_guard<std::mutex> lock(m_StageMutex);
			m_Stage = std::move(stage);
		}
		std::string GetStage() const {
			std::lock_guard<std::mutex> lock(m_StageMutex);
			return m_Stage;
		}

		// OnLoad debe revisar IsCancelled() entre fases (y pasarlo a los callbacks de
		// progreso) para que cerrar la app durante una carga no espere a que termine.
		void Cancel() { m_Cancelled.store(true, std::memory_order_relaxed); }
		bool IsCancelled() const { return m_Cancelled.load(std::memory_order_relaxed); }

		// Callback de progreso para una fase que ocupa [begin, end] de la barra global.
		// Devuelve false si la carga se canceló (mismo contrato que SVO::ProgressFn).
		auto Phase(float begin, float end) {
			return [this, begin, end](float fraction) {
				SetProgress(begin + (end - begin) * fraction);
				return !IsCancelled();
			};
		}
	};

}
