#ifdef LORE_PLATFORM_MAC

// Include the ImGui Metal backend implementation directly
// (compiled as ObjC++ in this .mm file)
// Must come before lrpch.h to avoid conflicts
#include <imgui.h>
#include <backends/imgui_impl_metal.mm>

#include "lrpch.h"
#include "ImGuiMetalBridge.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace Lore {

	namespace ImGuiMetalBridge {

		void Init(void* metalDevice) {
			id<MTLDevice> device = (__bridge id<MTLDevice>)metalDevice;
			ImGui_ImplMetal_Init(device);
		}

		void Shutdown() {
			ImGui_ImplMetal_Shutdown();
		}

		void NewFrame(void* passDescriptor) {
			MTLRenderPassDescriptor* desc = (__bridge MTLRenderPassDescriptor*)passDescriptor;
			ImGui_ImplMetal_NewFrame(desc);
		}

		void RenderDrawData(void* commandBuffer, void* renderEncoder) {
			id<MTLCommandBuffer> cmdBuf = (__bridge id<MTLCommandBuffer>)commandBuffer;
			id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)renderEncoder;
			ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cmdBuf, encoder);
		}

	}

}

#endif // LORE_PLATFORM_MAC
