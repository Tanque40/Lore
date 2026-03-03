#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalFramebuffer.h"
#include "MetalContext.h"

#import <Metal/Metal.h>

namespace Lore {

	MetalFramebuffer::MetalFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec) {
		Invalidate();
	}

	MetalFramebuffer::~MetalFramebuffer() {
		if (m_Texture) {
			CFRelease(m_Texture);
			m_Texture = nullptr;
		}
	}

	void MetalFramebuffer::Invalidate() {
		if (m_Texture) {
			CFRelease(m_Texture);
			m_Texture = nullptr;
		}

		MetalContext* ctx = MetalContext::Get();
		if (!ctx) return;

		id<MTLDevice> device = (__bridge id<MTLDevice>)ctx->GetDevice();

		MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
		desc.textureType = MTLTextureType2D;
		desc.width = m_Specification.Width;
		desc.height = m_Specification.Height;
		desc.pixelFormat = MTLPixelFormatBGRA8Unorm;
		desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
		desc.storageMode = MTLStorageModePrivate;

		id<MTLTexture> texture = [device newTextureWithDescriptor:desc];
		texture.label = @"LoreOffscreenFramebuffer";
		m_Texture = (__bridge_retained void*)texture;
	}

	void MetalFramebuffer::Bind() {
		MetalContext* ctx = MetalContext::Get();
		if (!ctx) return;
		ctx->SetOffscreenTexture(m_Texture);
	}

	void MetalFramebuffer::Unbind() {
		MetalContext* ctx = MetalContext::Get();
		if (!ctx) return;

		// End the offscreen render pass
		ctx->EndCurrentPass();
		// Clear the offscreen target
		ctx->ClearOffscreenTexture();
		// Start the screen render pass for ImGui
		ctx->BeginFrame();
	}

	void MetalFramebuffer::Resize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0 || width > 8192 || height > 8192) {
			LR_CORE_WARN("Attempted to resize Metal framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	void* MetalFramebuffer::GetColorAttachmentRendererID() const {
		return m_Texture;
	}

}

#endif // LORE_PLATFORM_MAC
