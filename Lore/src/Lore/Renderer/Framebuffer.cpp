#include "lrpch.h"

#include "Lore/Renderer/Framebuffer.h"
#include "Lore/Renderer/RendererAPI.h"

#ifdef LORE_PLATFORM_WINDOWS
#include "Lore/Platform/OpenGL/OpenGLFramebuffer.h"
#endif

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalFramebuffer.h"
#endif

namespace Lore {

	Framebuffer* Framebuffer::Create(const FramebufferSpecification& spec) {
#ifdef LORE_PLATFORM_WINDOWS
		return new OpenGLFramebuffer(spec);
#elif defined(LORE_PLATFORM_MAC)
		return new MetalFramebuffer(spec);
#else
		LR_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

}
