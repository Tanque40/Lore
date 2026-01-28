#pragma once

#include <memory>

#include "Core.h"
#include "spdlog/spdlog.h"

namespace Lore {

	class Log {
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	};

}


// * Core log macros
#define LR_CORE_ERROR(...) ::Lore::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LR_CORE_WARN(...)  ::Lore::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LR_CORE_INFO(...)  ::Lore::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LR_CORE_TRACE(...) ::Lore::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LR_CORE_FATAL(...) ::Lore::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// * Client log macros
#define LR_ERROR(...) ::Lore::Log::GetClientLogger()->error(__VA_ARGS__)
#define LR_WARN(...)  ::Lore::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LR_INFO(...)  ::Lore::Log::GetClientLogger()->info(__VA_ARGS__)
#define LR_TRACE(...) ::Lore::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LR_FATAL(...) ::Lore::Log::GetClientLogger()->fatal(__VA_ARGS__)
