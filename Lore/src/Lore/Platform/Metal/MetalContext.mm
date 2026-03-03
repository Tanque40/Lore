#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC
#include "MetalContext.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace Lore {

	MetalContext* MetalContext::s_Instance = nullptr;

	MetalContext::MetalContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle) {
		LR_CORE_ASSERT(windowHandle, "Window handle is null!");
		s_Instance = this;
	}

	MetalContext::~MetalContext() {
		if (s_Instance == this)
			s_Instance = nullptr;

		// Release retained Metal objects (those stored via __bridge_retained)
		if (m_BlitPipelineState) {
			CFRelease(m_BlitPipelineState);
			m_BlitPipelineState = nullptr;
		}
		if (m_CommandQueue) {
			CFRelease(m_CommandQueue);
			m_CommandQueue = nullptr;
		}
		if (m_Device) {
			CFRelease(m_Device);
			m_Device = nullptr;
		}
		// m_Layer, m_CurrentEncoder, m_CurrentCommandBuffer, m_CurrentDrawable
		// are weak references (__bridge void*), no need to release
		// m_CurrentComputePipeline, m_CurrentComputeTexture are also weak references
		m_CurrentEncoder = nullptr;
		m_CurrentCommandBuffer = nullptr;
		m_CurrentDrawable = nullptr;
		m_CurrentPassDescriptor = nullptr;
		m_CurrentComputePipeline = nullptr;
		m_CurrentComputeTexture = nullptr;
		m_Layer = nullptr;
	}

	void MetalContext::Init() {
		// Create Metal device
		id<MTLDevice> device = MTLCreateSystemDefaultDevice();
		LR_CORE_ASSERT(device, "Failed to create Metal device!");
		m_Device = (__bridge_retained void*)device;

		// Create command queue
		id<MTLCommandQueue> commandQueue = [device newCommandQueue];
		LR_CORE_ASSERT(commandQueue, "Failed to create Metal command queue!");
		m_CommandQueue = (__bridge_retained void*)commandQueue;

		// Get the native Cocoa window from GLFW
		NSWindow* nsWindow = glfwGetCocoaWindow(m_WindowHandle);
		LR_CORE_ASSERT(nsWindow, "Failed to get Cocoa window!");

		// Create and configure CAMetalLayer
		CAMetalLayer* metalLayer = [CAMetalLayer layer];
		metalLayer.device = device;
		metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		metalLayer.framebufferOnly = YES;

		// Get framebuffer size for proper retina scaling
		int width, height;
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);
		metalLayer.drawableSize = CGSizeMake(width, height);

		// Set the Metal layer as the window's content view layer
		nsWindow.contentView.wantsLayer = YES;
		nsWindow.contentView.layer = metalLayer;

		m_Layer = (__bridge void*)metalLayer;  // weak reference, layer is owned by the view

		LR_CORE_INFO("Metal Info:");
		LR_CORE_INFO("\tVendor: Apple");
		LR_CORE_INFO("\tDevice: {0}", [[device name] UTF8String]);
		if (@available(macOS 10.15, *)) {
			if ([device supportsFamily:MTLGPUFamilyApple1])
				LR_CORE_INFO("\tGPU Family: Apple Silicon");
			else if ([device supportsFamily:MTLGPUFamilyMac1])
				LR_CORE_INFO("\tGPU Family: Mac (Intel/AMD)");
		}
		LR_CORE_INFO("\tLow Power: {0}", [device isLowPower] ? "Yes" : "No");
		LR_CORE_INFO("\tMax Threads Per Threadgroup: {0}", (uint32_t)[device maxThreadsPerThreadgroup].width);
	}

	void MetalContext::BeginFrame() {
		id<MTLCommandQueue> commandQueue = (__bridge id<MTLCommandQueue>)m_CommandQueue;

		if (m_OffscreenTexture) {
			// Offscreen rendering: target the offscreen texture
			id<MTLTexture> offscreenTex = (__bridge id<MTLTexture>)m_OffscreenTexture;

			MTLRenderPassDescriptor* passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
			passDescriptor.colorAttachments[0].texture = offscreenTex;
			passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
			passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
			passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(
				m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a
			);
			m_CurrentPassDescriptor = (__bridge void*)passDescriptor;

			id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
			commandBuffer.label = @"LoreOffscreenCommandBuffer";
			m_CurrentCommandBuffer = (__bridge void*)commandBuffer;

			id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
			encoder.label = @"LoreOffscreenRenderEncoder";
			m_CurrentEncoder = (__bridge void*)encoder;

			// Set viewport to match offscreen texture size
			MTLViewport viewport;
			viewport.originX = 0;
			viewport.originY = 0;
			viewport.width = offscreenTex.width;
			viewport.height = offscreenTex.height;
			viewport.znear = 0.0;
			viewport.zfar = 1.0;
			[encoder setViewport:viewport];

			return;
		}

		// Screen rendering: acquire drawable and render to screen
		CAMetalLayer* layer = (__bridge CAMetalLayer*)m_Layer;

		// Update drawable size in case window was resized
		int width, height;
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);
		layer.drawableSize = CGSizeMake(width, height);

		// Get next drawable
		id<CAMetalDrawable> drawable = [layer nextDrawable];
		if (!drawable) {
			LR_CORE_ERROR("Failed to get next Metal drawable!");
			return;
		}
		m_CurrentDrawable = (__bridge void*)drawable;

		// Create render pass descriptor
		MTLRenderPassDescriptor* passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		passDescriptor.colorAttachments[0].texture = drawable.texture;
		passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(
			m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a
		);
		m_CurrentPassDescriptor = (__bridge void*)passDescriptor;

		// Create command buffer
		id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
		commandBuffer.label = @"LoreCommandBuffer";
		m_CurrentCommandBuffer = (__bridge void*)commandBuffer;

		// Create render command encoder
		id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
		encoder.label = @"LoreRenderEncoder";
		m_CurrentEncoder = (__bridge void*)encoder;
	}

	void MetalContext::EndCurrentPass() {
		if (!m_CurrentEncoder || !m_CurrentCommandBuffer)
			return;

		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_CurrentEncoder;
		id<MTLCommandBuffer> commandBuffer = (__bridge id<MTLCommandBuffer>)m_CurrentCommandBuffer;

		[encoder endEncoding];
		[commandBuffer commit];
		[commandBuffer waitUntilCompleted];

		m_CurrentEncoder = nullptr;
		m_CurrentCommandBuffer = nullptr;
		m_CurrentPassDescriptor = nullptr;
	}

	void MetalContext::SwapBuffers() {
		if (!m_CurrentEncoder || !m_CurrentCommandBuffer || !m_CurrentDrawable)
			return;

		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_CurrentEncoder;
		id<MTLCommandBuffer> commandBuffer = (__bridge id<MTLCommandBuffer>)m_CurrentCommandBuffer;
		id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)m_CurrentDrawable;

		// End encoding
		[encoder endEncoding];

		// Present and commit
		[commandBuffer presentDrawable:drawable];
		[commandBuffer commit];
		[commandBuffer waitUntilCompleted];

		// Reset frame state
		m_CurrentEncoder = nullptr;
		m_CurrentCommandBuffer = nullptr;
		m_CurrentDrawable = nullptr;
		m_CurrentPassDescriptor = nullptr;
	}

	void* MetalContext::GetBlitPipelineState() {
		if (m_BlitPipelineState) return m_BlitPipelineState;

		id<MTLDevice> device = (__bridge id<MTLDevice>)m_Device;

		// Inline MSL blit shader: full-screen triangle using vertex_id
		NSString* blitShaderSource = @R"(
			#include <metal_stdlib>
			using namespace metal;

			struct BlitOut {
				float4 position [[position]];
				float2 texCoord;
			};

			vertex BlitOut blit_vertex(uint vid [[vertex_id]]) {
				BlitOut out;
				out.texCoord = float2((vid << 1) & 2, vid & 2);
				out.position = float4(out.texCoord * 2.0 - 1.0, 0.0, 1.0);
				return out;
			}

			fragment float4 blit_fragment(BlitOut in [[stage_in]], texture2d<float> tex [[texture(0)]]) {
				constexpr sampler s(filter::nearest);
				return tex.sample(s, in.texCoord);
			}
		)";

		NSError* error = nil;
		MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
		id<MTLLibrary> library = [device newLibraryWithSource:blitShaderSource options:options error:&error];

		if (error) {
			LR_CORE_ERROR("Failed to compile Metal blit shader: {0}",
				std::string([[error localizedDescription] UTF8String]));
			return nullptr;
		}

		id<MTLFunction> vertexFunction = [library newFunctionWithName:@"blit_vertex"];
		id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"blit_fragment"];

		LR_CORE_ASSERT(vertexFunction, "Failed to find blit_vertex in blit shader!");
		LR_CORE_ASSERT(fragmentFunction, "Failed to find blit_fragment in blit shader!");

		// Create pipeline without vertex descriptor (vertex_id only)
		MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
		desc.vertexFunction = vertexFunction;
		desc.fragmentFunction = fragmentFunction;
		desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

		error = nil;
		id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:desc error:&error];

		if (error) {
			LR_CORE_ERROR("Failed to create Metal blit pipeline state: {0}",
				std::string([[error localizedDescription] UTF8String]));
			return nullptr;
		}

		m_BlitPipelineState = (__bridge_retained void*)pipelineState;
		LR_CORE_INFO("Metal blit pipeline created successfully.");
		return m_BlitPipelineState;
	}

}

#endif // LORE_PLATFORM_MAC
