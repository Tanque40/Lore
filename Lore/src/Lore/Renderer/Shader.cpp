#include "lrpch.h"
#include "Shader.h"
#include "RendererAPI.h"

#include "Lore/Platform/OpenGL/OpenGLShader.h"

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalShader.h"
#endif

namespace Lore {

	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc) {
		switch (RendererAPI::GetAPI()) {
		case RendererAPIType::OpenGL: return new OpenGLShader(vertexSrc, fragmentSrc);
#ifdef LORE_PLATFORM_MAC
		case RendererAPIType::Metal:  return new MetalShader(vertexSrc, fragmentSrc);
#endif
		default:
			LR_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

}
