#pragma once

#ifdef LORE_PLATFORM_WINDOWS
#ifdef LORE_BUILD_DLL
#define LORE_API __declspec(dllexport)
#else
#define LORE_API __declspec(dllimport)
#endif
#elif defined(LORE_PLATFORM_MAC)
#include <csignal>
#define LORE_API
#else
#error Lore only supports Windows or Mac!
#endif

#ifdef LORE_ENABLE_ASSERTS
#ifdef LORE_PLATFORM_WINDOWS
#define LR_ASSERT(x, ...) { if(!(x)) { LR_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define LR_CORE_ASSERT(x, ...) { if(!(x)) { LR_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#elif defined(LORE_PLATFORM_MAC)
#define LR_ASSERT(x, ...) { if(!(x)) { LR_ERROR("Assertion Failed: {0}", __VA_ARGS__); __builtin_trap(); } }
#define LR_CORE_ASSERT(x, ...) { if(!(x)) { LR_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __builtin_trap(); } }
#endif
#else
#define LR_ASSERT(x, ...)
#define LR_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
