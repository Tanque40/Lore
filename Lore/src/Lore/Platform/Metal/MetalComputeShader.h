#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/ComputeShader.h"

#include <glm/glm.hpp>

namespace Lore {

	// TODO figuring out a better way to handle uniforms in Metal compute shaders, since we can't set them directly like in OpenGL. For now, we'll just store them in a struct and pass a pointer to it as a buffer.
	class MetalComputeShader : public ComputeShader {

	public:

		// Padded to match Metal float3 alignment (16 bytes per float3)
		struct Uniforms {
			glm::vec3 cameraPos;
			float _pad0 = 0;
			glm::vec3 cameraDir;
			float _pad1 = 0;
			glm::vec3 cameraUp;
			float _pad2 = 0;
			glm::vec3 cameraRight;
			float _pad3 = 0;
			float fov;
		};


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

		Uniforms m_Uniforms;
	};

}

#endif // LORE_PLATFORM_MAC
