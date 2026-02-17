#pragma once

#ifdef LORE_PLATFORM_MAC

namespace Lore {

	// Helper functions to call ImGui Metal backend from C++ code
	// Implementation in ImGuiMetalHelper.mm (compiled as ObjC++)
	void ImGuiMetalHelper_Init();
	void ImGuiMetalHelper_Shutdown();
	void ImGuiMetalHelper_NewFrame();
	void ImGuiMetalHelper_RenderDrawData();

}

#endif
