#include "lrpch.h"

#include "Lore/Core/JobSystem.h"

#include <deque>

namespace Lore {

	namespace Detail {

		struct JobState {
			std::atomic<bool> Done{ false };
			std::mutex Mutex;
			std::condition_variable Finished;
		};

	}

	namespace {

		struct QueuedJob {
			std::function<void()> Work;
			std::shared_ptr<Detail::JobState> State;
		};

		struct Pool {
			std::mutex Mutex;
			std::condition_variable HasWork;
			std::deque<QueuedJob> Queue;
			std::vector<std::thread> Workers;
			bool Stopping = false;
		};

		// Puntero para poder reiniciar el pool (Shutdown + Init) en las pruebas.
		std::unique_ptr<Pool> s_Pool;
		std::mutex s_LifecycleMutex;

		thread_local bool t_IsWorker = false;

		void Execute(QueuedJob& job) {
			job.Work();
			{
				std::lock_guard<std::mutex> lock(job.State->Mutex);
				job.State->Done.store(true, std::memory_order_release);
			}
			job.State->Finished.notify_all();
		}

		void WorkerLoop(Pool* pool) {
			t_IsWorker = true;
			for (;;) {
				QueuedJob job;
				{
					std::unique_lock<std::mutex> lock(pool->Mutex);
					pool->HasWork.wait(lock, [pool] { return pool->Stopping || !pool->Queue.empty(); });
					if (pool->Queue.empty())
						return; // Stopping y sin trabajo pendiente
					job = std::move(pool->Queue.front());
					pool->Queue.pop_front();
				}
				Execute(job);
			}
		}

		Pool& EnsurePool() {
			if (!s_Pool)
				JobSystem::Init();
			return *s_Pool;
		}

	}

	bool JobHandle::IsDone() const {
		return !m_State || m_State->Done.load(std::memory_order_acquire);
	}

	void JobHandle::Wait() const {
		if (!m_State)
			return;

		if (t_IsWorker) {
			// Dentro del pool: ayudar en vez de bloquear un worker.
			while (!IsDone()) {
				if (!JobSystem::RunPendingJob())
					std::this_thread::yield();
			}
			return;
		}

		std::unique_lock<std::mutex> lock(m_State->Mutex);
		m_State->Finished.wait(lock, [this] { return m_State->Done.load(std::memory_order_acquire); });
	}

	void JobSystem::Init(uint32_t workerCount) {
		std::lock_guard<std::mutex> lifecycle(s_LifecycleMutex);
		if (s_Pool)
			return;

		if (workerCount == 0) {
			uint32_t hardware = std::thread::hardware_concurrency();
			workerCount = hardware > 1 ? hardware - 1 : 1;
		}

		s_Pool = std::make_unique<Pool>();
		s_Pool->Workers.reserve(workerCount);
		for (uint32_t i = 0; i < workerCount; i++)
			s_Pool->Workers.emplace_back(WorkerLoop, s_Pool.get());
	}

	void JobSystem::Shutdown() {
		std::lock_guard<std::mutex> lifecycle(s_LifecycleMutex);
		if (!s_Pool)
			return;

		{
			std::lock_guard<std::mutex> lock(s_Pool->Mutex);
			s_Pool->Stopping = true;
		}
		s_Pool->HasWork.notify_all();
		for (std::thread& worker : s_Pool->Workers)
			worker.join();
		s_Pool.reset();
	}

	bool JobSystem::IsInitialized() {
		return s_Pool != nullptr;
	}

	uint32_t JobSystem::GetWorkerCount() {
		return static_cast<uint32_t>(EnsurePool().Workers.size());
	}

	JobHandle JobSystem::Submit(std::function<void()> job) {
		Pool& pool = EnsurePool();
		auto state = std::make_shared<Detail::JobState>();
		{
			std::lock_guard<std::mutex> lock(pool.Mutex);
			pool.Queue.push_back({ std::move(job), state });
		}
		pool.HasWork.notify_one();
		return JobHandle(state);
	}

	bool JobSystem::RunPendingJob() {
		Pool& pool = EnsurePool();
		QueuedJob job;
		{
			std::lock_guard<std::mutex> lock(pool.Mutex);
			if (pool.Queue.empty())
				return false;
			job = std::move(pool.Queue.front());
			pool.Queue.pop_front();
		}
		Execute(job);
		return true;
	}

	void JobSystem::ParallelFor(uint32_t count, uint32_t grain, const std::function<void(uint32_t, uint32_t)>& fn) {
		if (count == 0)
			return;
		grain = std::max(1u, grain);
		uint32_t blocks = (count + grain - 1) / grain;

		if (blocks == 1) {
			fn(0, count);
			return;
		}

		// Los bloques se reparten con un contador atómico: cada participante (los helpers
		// encolados y el hilo que llama) toma el siguiente bloque libre hasta agotarlos.
		// El que llama sólo espera a que se COMPLETEN los bloques, no a que los helpers
		// arranquen, así que si el pool está lleno simplemente hace todo el trabajo él.
		struct Shared {
			std::atomic<uint32_t> Next{ 0 };
			std::atomic<uint32_t> Completed{ 0 };
		};
		auto shared = std::make_shared<Shared>();

		auto worker = [shared, blocks, grain, count, &fn]() {
			for (;;) {
				uint32_t block = shared->Next.fetch_add(1, std::memory_order_relaxed);
				if (block >= blocks)
					return;
				uint32_t begin = block * grain;
				uint32_t end = std::min(count, begin + grain);
				fn(begin, end);
				shared->Completed.fetch_add(1, std::memory_order_acq_rel);
			}
		};

		// `fn` se captura por referencia: es seguro porque esta función no regresa hasta
		// que todos los bloques terminaron, y un helper que arranca tarde ve Next >= blocks
		// y sale sin tocar fn.
		uint32_t helpers = std::min(GetWorkerCount(), blocks - 1);
		for (uint32_t i = 0; i < helpers; i++)
			Submit(worker);

		worker();

		// Aquí ya no quedan bloques sin asignar: sólo falta que terminen los que otros
		// helpers tienen en curso. No tomamos trabajos ajenos de la cola mientras tanto
		// (podría ser otro trabajo largo y retrasaría este ParallelFor).
		while (shared->Completed.load(std::memory_order_acquire) < blocks)
			std::this_thread::yield();
	}

}
