#include "lrpch.h"
#include "Lore/Renderer/RenderCommand.h"

#ifdef LORE_PLATFORM_WINDOWS
#include "Lore/Platform/OpenGL/OpenGLRendererAPI.h"
#endif

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalRendererAPI.h"
#endif

namespace Lore {

#ifdef LORE_PLATFORM_MAC

	RendererAPI* RenderCommand::s_RendererAPI = new MetalRendererAPI();

#elif defined(LORE_PLATFORM_WINDOWS)

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();

#endif

}
