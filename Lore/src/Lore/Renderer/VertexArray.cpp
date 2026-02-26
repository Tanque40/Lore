#include "lrpch.h"

#include "Lore/Renderer/VertexArray.h"

#include "Lore/Renderer/Renderer.h"

#include "Lore/Platform/OpenGL/OpenGLVertexArray.h"

namespace Lore {

	VertexArray* VertexArray::Create() {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			LR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexArray();
			break;
		}

		LR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
