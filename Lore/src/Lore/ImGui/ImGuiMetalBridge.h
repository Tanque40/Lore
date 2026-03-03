#pragma once

#ifdef LORE_PLATFORM_MAC

// C++ bridge for ImGui Metal backend calls.
// This avoids including Objective-C headers in .cpp files.

namespace Lore {

	namespace ImGuiMetalBridge {
		void Init(void* metalDevice);      // id<MTLDevice> as void*
		void Shutdown();
		void NewFrame(void* passDescriptor); // MTLRenderPassDescriptor* as void*
		void RenderDrawData(void* commandBuffer, void* renderEncoder); // id<MTLCommandBuffer>, id<MTLRenderCommandEncoder>
	}

}

#endif // LORE_PLATFORM_MAC
