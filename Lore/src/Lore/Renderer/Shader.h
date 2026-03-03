#pragma once

#include <glm/glm.hpp>

namespace Lore {

	class Shader {
	public:
		virtual ~Shader() {};

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		// * Set uniforms **********

		// ? Floats
		virtual void SetUniform1f(const std::string& name, float value) = 0;
		virtual void SetUniform2f(const std::string& name, const glm::vec2& vector) = 0;
		virtual void SetUniform3f(const std::string& name, const glm::vec3& vector) = 0;
		virtual void SetUniform4f(const std::string& name, const glm::vec4& vector) = 0;

		// ? Matrices
		virtual void SetUniformMat3f(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) = 0;

		// * Set uniforms **********

		static Shader* Create(const std::string& vertexPath, const std::string& fragmentPath, const std::string& computeShaderPath);

	protected:
		std::string ReadFile(const std::string& filePath);
	};

}
