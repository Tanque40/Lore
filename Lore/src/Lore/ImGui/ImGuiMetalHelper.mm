#ifdef LORE_PLATFORM_MAC

#include "ImGuiMetalHelper.h"
#include "Lore/Platform/Metal/MetalContext.h"

#include <imgui.h>
#include <backends/imgui_impl_metal.h>
#include <backends/imgui_impl_glfw.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace Lore {

	void ImGuiMetalHelper_Init() {
		id<MTLDevice> device = (__bridge id<MTLDevice>)MetalContext::Get()->GetDevice();
		ImGui_ImplMetal_Init(device);
	}

	void ImGuiMetalHelper_Shutdown() {
		ImGui_ImplMetal_Shutdown();
	}

	void ImGuiMetalHelper_NewFrame() {
		MTLRenderPassDescriptor* passDesc = (__bridge MTLRenderPassDescriptor*)MetalContext::Get()->GetCurrentPassDescriptor();
		ImGui_ImplMetal_NewFrame(passDesc);
	}

	void ImGuiMetalHelper_RenderDrawData() {
		id<MTLCommandBuffer> commandBuffer = (__bridge id<MTLCommandBuffer>)MetalContext::Get()->GetCurrentCommandBuffer();
		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)MetalContext::Get()->GetCurrentEncoder();
		ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, encoder);
	}

}

#endif
