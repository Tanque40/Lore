#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

namespace Lore {

	namespace Detail { struct JobState; }

	// Referencia a un trabajo enviado al JobSystem. Copiable; un handle vacío cuenta como
	// terminado.
	class JobHandle {
	private:
		std::shared_ptr<Detail::JobState> m_State;

	public:
		JobHandle() = default;
		explicit JobHandle(std::shared_ptr<Detail::JobState> state) : m_State(std::move(state)) {}

		bool IsValid() const { return m_State != nullptr; }
		bool IsDone() const;
		// Bloquea hasta que el trabajo termine. Si se llama desde un hilo del pool, ese hilo
		// ayuda a vaciar la cola mientras espera (no se queda dormido ocupando un worker).
		void Wait() const;
	};

	// Pool de hilos de propósito general. Pensado para trabajo de CPU largo (generar el
	// mundo, construir el SVO) que no debe congelar el hilo principal. Nada de lo que corre
	// aquí puede tocar la API gráfica: OpenGL no admite llamadas fuera del hilo del contexto
	// y en Metal no lo necesitamos. Subir a la GPU siempre se hace después, en el hilo
	// principal (ver Scene::OnAttach).
	class JobSystem {
	public:
		// workerCount = 0 usa (núcleos - 1), con mínimo 1. Application lo llama al arrancar;
		// si alguien usa el sistema sin inicializarlo (p. ej. las pruebas) se inicializa solo.
		static void Init(uint32_t workerCount = 0);
		// Termina los trabajos ya encolados y une los hilos.
		static void Shutdown();
		static bool IsInitialized();

		static uint32_t GetWorkerCount();

		static JobHandle Submit(std::function<void()> job);

		// Ejecuta fn(begin, end) sobre [0, count) partido en bloques de `grain` elementos y
		// bloquea hasta que todos terminen. El hilo que llama también procesa bloques, así
		// que es seguro llamarlo desde dentro de un trabajo (no hay deadlock aunque todos
		// los workers estén ocupados).
		static void ParallelFor(uint32_t count, uint32_t grain, const std::function<void(uint32_t begin, uint32_t end)>& fn);

	private:
		// Ejecuta un trabajo pendiente si hay alguno. Devuelve false si la cola estaba vacía.
		static bool RunPendingJob();

		friend class JobHandle;
	};

}
