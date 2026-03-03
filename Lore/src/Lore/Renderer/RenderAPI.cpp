#include "lrpch.h"

#include "Lore/Renderer/RendererAPI.h"

namespace Lore {

#ifdef LORE_PLATFORM_WINDOWS

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

#endif

#ifdef LORE_PLATFORM_MAC
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Metal;
#endif

}
