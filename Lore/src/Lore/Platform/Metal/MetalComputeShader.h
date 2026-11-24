#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/ComputeShader.h"

#include <glm/glm.hpp>

namespace Lore {

	// Carga el .metal que genera Scripts/CompileShaders.py (spirv-cross) a partir de la fuente
	// GLSL única. Los datos por frame llegan en un UniformBuffer, no con SetUniform*.
	class MetalComputeShader : public ComputeShader {

	public:
		MetalComputeShader(const std::string& computePath);
		virtual ~MetalComputeShader();

		virtual void Bind() override;
		virtual void Unbind() override;

		void* GetComputePipelineState() const { return m_ComputePipelineState; }

		// * Set uniforms **********

		// ? Floats
		virtual void SetUniform1f(const std::string& name, float value) override;
		virtual void SetUniform2f(const std::string& name, const glm::vec2& vector) override;
		virtual void SetUniform3f(const std::string& name, const glm::vec3& vector) override;
		virtual void SetUniform4f(const std::string& name, const glm::vec4& vector) override;

		// ? Matrices
		virtual void SetUniformMat3f(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) override;

	private:
		void* m_Library = nullptr;               // id<MTLLibrary>
		void* m_ComputePipelineState = nullptr;  // id<MTLComputePipelineState>
	};

}

#endif // LORE_PLATFORM_MAC
