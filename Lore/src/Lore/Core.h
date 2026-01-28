#pragma once

#ifdef LORE_PLATFORM_WINDOWS
#ifdef LORE_BUILD_DLL
#define LORE_API __declspec(dllexport)
#else
#define LORE_API __declspec(dllimport)
#endif
#elif defined(LORE_PLATFORM_MAC)
#define LORE_API
#else
#error Lore only supports Windows!
#endif
