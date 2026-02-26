#include "lrpch.h"
#include "Lore/Renderer/RenderCommand.h"

#include "Lore/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Lore {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();

}
