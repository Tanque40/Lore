#include "lrpch.h"

#ifdef LORE_PLATFORM_WINDOWS

#include "Lore/Platform/OpenGL/OpenGLComputeShader.h"

#include <glm/gtc/type_ptr.hpp>

namespace Lore {

	OpenGLComputeShader::OpenGLComputeShader(const std::string& computePath) {
		std::string source = ReadFile(computePath);

		// Compile the compute shader
		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
		const GLchar* src = source.c_str();
		glShaderSource(computeShader, 1, &src, 0);
		glCompileShader(computeShader);

		GLint isCompiled = 0;
		glGetShaderiv(computeShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(computeShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(computeShader);
			LR_CORE_ERROR("Compute shader compilation failed: {0}", infoLog.data());
			LR_CORE_ASSERT(false, "Compute shader compilation failed!");
			return;
		}

		// Create a separate program for the compute shader
		// (OpenGL requires compute shaders to be in their own program)
		m_ProgramID = glCreateProgram();
		glAttachShader(m_ProgramID, computeShader);
		glLinkProgram(m_ProgramID);

		GLint isLinked = 0;
		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(m_ProgramID, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_ProgramID, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(m_ProgramID);
			glDeleteShader(computeShader);
			m_ProgramID = 0;

			LR_CORE_ERROR("Compute shader program linking failed: {0}", infoLog.data());
			LR_CORE_ASSERT(false, "Compute shader program linking failed!");
			return;
		}

		glDetachShader(m_ProgramID, computeShader);
		glDeleteShader(computeShader);
	}

	OpenGLComputeShader::~OpenGLComputeShader() {
		if (m_ProgramID) {
			glDeleteProgram(m_ProgramID);
		}
	}

	void OpenGLComputeShader::Bind() {
		glUseProgram(m_ProgramID);
	}

	void OpenGLComputeShader::Unbind() {
		glUseProgram(0);
	}

	void OpenGLComputeShader::SetUniform1f(const std::string& name, float value) {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLComputeShader::SetUniform2f(const std::string& name, const glm::vec2& vector) {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform2fv(location, 1, glm::value_ptr(vector));
	}

	void OpenGLComputeShader::SetUniform3f(const std::string& name, const glm::vec3& vector) {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform3fv(location, 1, glm::value_ptr(vector));
	}

	void OpenGLComputeShader::SetUniform4f(const std::string& name, const glm::vec4& vector) {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform4fv(location, 1, glm::value_ptr(vector));
	}

	void OpenGLComputeShader::SetUniformMat3f(const std::string& name, const glm::mat3& matrix) {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLComputeShader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) {
		GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}

#endif
