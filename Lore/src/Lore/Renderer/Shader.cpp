#include "lrpch.h"

#include "Shader.h"

#include "Lore/Renderer/Renderer.h"

#include "Lore/Platform/OpenGL/OpenGLShader.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace Lore {

	Shader* Shader::Create(const std::string& vertexPath, const std::string& fragmentPath, const std::string& computeShaderPath) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			LR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLShader(vertexPath, fragmentPath, computeShaderPath);
			break;
		}

		LR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
