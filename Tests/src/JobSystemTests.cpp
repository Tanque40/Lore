#include "lrpch.h"

#include "TestCommon.h"

#include "Lore/Core/JobSystem.h"
#include "Lore/Scene/LoadContext.h"

void RunJobSystemTests() {
	using Lore::JobSystem;

	// Cada prueba con 1 worker (el caso que más fácil se traba) y con varios.
	for (uint32_t workers : { 1u, 4u }) {
		JobSystem::Shutdown();
		JobSystem::Init(workers);
		std::string tag = " (" + std::to_string(workers) + " workers)";
		Check(JobSystem::GetWorkerCount() == workers, "GetWorkerCount" + tag);

		// Submit + Wait
		std::atomic<int> counter{ 0 };
		std::vector<Lore::JobHandle> handles;
		for (int i = 0; i < 100; i++)
			handles.push_back(JobSystem::Submit([&counter] { counter.fetch_add(1); }));
		for (auto& handle : handles) handle.Wait();
		bool allDone = true;
		for (auto& handle : handles) allDone = allDone && handle.IsDone();
		Check(counter.load() == 100 && allDone, "Submit/Wait ejecuta los 100 trabajos" + tag);
		Check(Lore::JobHandle().IsDone(), "un handle vacío cuenta como terminado" + tag);

		// ParallelFor cubre cada índice exactamente una vez, con bloques de varios tamaños.
		for (uint32_t grain : { 1u, 7u, 1000u, 5000u }) {
			std::vector<std::atomic<int>> hits(4321);
			JobSystem::ParallelFor(4321, grain, [&](uint32_t begin, uint32_t end) {
				for (uint32_t i = begin; i < end; i++) hits[i].fetch_add(1);
				});
			bool exactlyOnce = true;
			for (auto& hit : hits) exactlyOnce = exactlyOnce && hit.load() == 1;
			Check(exactlyOnce, "ParallelFor grain " + std::to_string(grain) + " visita cada índice una vez" + tag);
		}

		// ParallelFor dentro de un trabajo: con 1 worker ocupado por el trabajo externo, el
		// interno tiene que avanzar igual (lo hace el propio hilo que llama).
		std::atomic<uint64_t> sum{ 0 };
		Lore::JobHandle outer = JobSystem::Submit([&sum] {
			JobSystem::ParallelFor(1000, 10, [&sum](uint32_t begin, uint32_t end) {
				uint64_t local = 0;
				for (uint32_t i = begin; i < end; i++) local += i;
				sum.fetch_add(local);
				});
			});
		outer.Wait();
		Check(sum.load() == 999ull * 1000ull / 2ull, "ParallelFor anidado en un trabajo no se traba" + tag);

		// Un trabajo que espera a otro trabajo encolado detrás de él (1 worker): Wait desde
		// un worker ayuda a vaciar la cola en vez de bloquearse.
		std::atomic<bool> innerRan{ false };
		Lore::JobHandle waiter = JobSystem::Submit([&innerRan] {
			Lore::JobHandle inner = JobSystem::Submit([&innerRan] { innerRan = true; });
			inner.Wait();
			});
		waiter.Wait();
		Check(innerRan.load(), "Wait dentro de un worker no se traba" + tag);
	}

	// LoadContext: fases, progreso monotónico y cancelación.
	Lore::LoadContext context;
	auto phase = context.Phase(0.5f, 0.9f);
	Check(phase(0.5f), "Phase devuelve true sin cancelar");
	Check(std::abs(context.GetProgress() - 0.7f) < 1e-5f, "Phase(0.5, 0.9)(0.5) = 0.7");
	context.SetProgress(0.2f);
	Check(std::abs(context.GetProgress() - 0.7f) < 1e-5f, "el progreso no retrocede");
	context.Cancel();
	Check(!phase(0.6f), "Phase devuelve false tras Cancel");
}
