#include "lrpch.h"

#include "Lore/Renderer/ComputeShader.h"
#include "Lore/Renderer/Renderer.h"

#ifdef LORE_PLATFORM_WINDOWS
#include "Lore/Platform/OpenGL/OpenGLComputeShader.h"
#endif

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalComputeShader.h"
#endif

namespace Lore {

	ComputeShader* ComputeShader::Create(const std::string& computePath) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			LR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
#ifdef LORE_PLATFORM_WINDOWS
		case RendererAPI::API::OpenGL:
			return new OpenGLComputeShader(computePath);
			break;
#endif
#ifdef LORE_PLATFORM_MAC
		case RendererAPI::API::Metal:
			return new MetalComputeShader(computePath);
			break;
#endif
		}

		LR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::string ComputeShader::ReadFile(const std::string& filePath) {
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
