#pragma once

#include "Lore/Renderer/Shader.h"

#include <glad/glad.h>

namespace Lore {

	class OpenGLShader : public Shader {
	private:
		uint32_t m_RendererID;

	public:
		OpenGLShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& computeShaderPath);
		virtual ~OpenGLShader();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetUniform1f(const std::string& name, float value) override;
		virtual void SetUniform2f(const std::string& name, const glm::vec2& vector) override;
		virtual void SetUniform3f(const std::string& name, const glm::vec3& vector) override;
		virtual void SetUniform4f(const std::string& name, const glm::vec4& vector) override;

		virtual void SetUniformMat3f(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) override;

	private:
		GLuint CompileVertexShader(const std::string& vertexSrc);
		GLuint CompileFragmentShader(const std::string& fragmentSrc);
		GLuint CompileComputeShader(const std::string& computeSrc);
		std::string ReadFile(const std::string& filePath);
	};

}
