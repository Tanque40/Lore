#include "lrpch.h"

#include "Shader.h"

#include "Lore/Renderer/Renderer.h"

#ifdef LORE_PLATFORM_WINDOWS
#include "Lore/Platform/OpenGL/OpenGLShader.h"
#endif

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalShader.h"
#endif

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace Lore {

	Shader* Shader::Create(const std::string& vertexPath, const std::string& fragmentPath, const std::string& computeShaderPath) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			LR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
#ifdef LORE_PLATFORM_WINDOWS
		case RendererAPI::API::OpenGL:
			return new OpenGLShader(vertexPath, fragmentPath, computeShaderPath);
			break;
#endif
#ifdef LORE_PLATFORM_MAC
		case RendererAPI::API::Metal:
			return new MetalShader(vertexPath, fragmentPath, computeShaderPath);
			break;
#endif
		}

		LR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::string Shader::ReadFile(const std::string& filePath) {
		std::string result;
		std::ifstream in(filePath, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else {
			LR_CORE_ERROR("Could not open file: {0}", filePath);
		}
		return result;
	}
}
