#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalComputeTexture.h"
#include "MetalContext.h"

#import <Metal/Metal.h>

namespace Lore {

	MetalComputeTexture::MetalComputeTexture(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height) {
		Invalidate();
	}

	MetalComputeTexture::~MetalComputeTexture() {
		if (m_Texture) {
			CFRelease(m_Texture);
			m_Texture = nullptr;
		}
	}

	void MetalComputeTexture::Invalidate() {
		if (m_Texture) {
			CFRelease(m_Texture);
			m_Texture = nullptr;
		}

		MetalContext* ctx = MetalContext::Get();
		if (!ctx) return;

		id<MTLDevice> device = (__bridge id<MTLDevice>)ctx->GetDevice();

		MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
		desc.textureType = MTLTextureType2D;
		desc.width = m_Width;
		desc.height = m_Height;
		desc.pixelFormat = MTLPixelFormatRGBA8Unorm;
		desc.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
		desc.storageMode = MTLStorageModePrivate;

		id<MTLTexture> texture = [device newTextureWithDescriptor:desc];
		texture.label = @"LoreComputeTexture";
		m_Texture = (__bridge_retained void*)texture;
	}

	void MetalComputeTexture::BindAsImage(uint32_t unit) {
		// For Metal, store the texture in MetalContext so DispatchCompute can access it.
		MetalContext* ctx = MetalContext::Get();
		if (ctx) {
			ctx->SetCurrentComputeTexture(m_Texture);
		}
	}

	void MetalComputeTexture::BindAsTexture(uint32_t slot) {
		// For Metal, texture binding for sampling is handled directly in BlitToScreen
		// via the native handle. No global state to set here.
	}

	void MetalComputeTexture::Resize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0 || width > 8192 || height > 8192) {
			LR_CORE_WARN("Attempted to resize Metal compute texture to {0}, {1}", width, height);
			return;
		}

		m_Width = width;
		m_Height = height;
		Invalidate();
	}

}

#endif // LORE_PLATFORM_MAC
