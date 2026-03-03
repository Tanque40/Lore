#include "lrpch.h"

#include <backends/imgui_impl_glfw.cpp>

#ifdef LORE_PLATFORM_WINDOWS
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_opengl3_loader.h>
#include <backends/imgui_impl_opengl3.cpp>
#endif

// NOTE: imgui_impl_metal.mm is compiled through ImGuiMetalBridge.mm (ObjC++ context)
