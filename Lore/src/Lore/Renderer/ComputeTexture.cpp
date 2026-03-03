#include "lrpch.h"

#include "Lore/Renderer/ComputeTexture.h"
#include "Lore/Renderer/Renderer.h"

#ifdef LORE_PLATFORM_WINDOWS
#include "Lore/Platform/OpenGL/OpenGLComputeTexture.h"
#endif

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalComputeTexture.h"
#endif

namespace Lore {

	ComputeTexture* ComputeTexture::Create(uint32_t width, uint32_t height) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			LR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
#ifdef LORE_PLATFORM_WINDOWS
		case RendererAPI::API::OpenGL:
			return new OpenGLComputeTexture(width, height);
			break;
#endif
#ifdef LORE_PLATFORM_MAC
		case RendererAPI::API::Metal:
			return new MetalComputeTexture(width, height);
			break;
#endif
		}

		LR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
