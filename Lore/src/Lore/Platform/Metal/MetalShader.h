#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/Shader.h"

#include <glm/glm.hpp>

namespace Lore {

	class MetalShader : public Shader {
	public:
		MetalShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& computeShaderPath);
		virtual ~MetalShader();

		virtual void Bind() override;
		virtual void Unbind() override;

		// * Set uniforms **********

		// ? Floats
		virtual void SetUniform1f(const std::string& name, float value) override;
		virtual void SetUniform2f(const std::string& name, const glm::vec2& vector) override;
		virtual void SetUniform3f(const std::string& name, const glm::vec3& vector) override;
		virtual void SetUniform4f(const std::string& name, const glm::vec4& vector) override;

		// ? Matrices
		virtual void SetUniformMat3f(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) override;

		void* GetPipelineState() const { return m_PipelineState; }

	private:
		struct Uniforms {
			glm::mat4 ViewProjection{ 1.0f };
			glm::mat4 Transform{ 1.0f };
		};

		void* m_Library = nullptr;       // id<MTLLibrary>
		void* m_PipelineState = nullptr; // id<MTLRenderPipelineState>
		Uniforms m_Uniforms;
	};

}
#endif
