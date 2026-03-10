#include "lrpch.h"

#ifdef LORE_PLATFORM_WINDOWS

#include "Lore/Platform/OpenGL/OpenGLShader.h"


#include <glm/gtc/type_ptr.hpp>

namespace Lore {

	OpenGLShader::OpenGLShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& computeShaderPath) {
		GLuint vertexShader, fragmentShader, computeShader;
		if (!vertexPath.empty()) {
			std::string vertexSource = ReadFile(vertexPath);
			vertexShader = CompileVertexShader(vertexSource);
		}
		if (!fragmentPath.empty()) {
			std::string fragmentSource = ReadFile(fragmentPath);
			fragmentShader = CompileFragmentShader(fragmentSource);
		}
		if (!computeShaderPath.empty()) {
			std::string computeSource = ReadFile(computeShaderPath);
			computeShader = CompileComputeShader(computeSource);
		}

		m_RendererID = glCreateProgram();

		if (!vertexPath.empty())
			glAttachShader(m_RendererID, vertexShader);
		if (!fragmentPath.empty())
			glAttachShader(m_RendererID, fragmentShader);
		if (!computeShaderPath.empty())
			glAttachShader(m_RendererID, computeShader);

		glLinkProgram(m_RendererID);

		GLint isLinked = 0;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(m_RendererID);
			LR_CORE_ERROR("Shader program linking failed: {0}", infoLog.data());
			LR_ASSERT(false, "Shader program linking failed!");

			if (!vertexPath.empty())
				glDeleteShader(vertexShader);
			if (!fragmentPath.empty())
				glDeleteShader(fragmentShader);
			if (!computeShaderPath.empty())
				glDeleteShader(computeShader);

			return;
		}

		if (!vertexPath.empty())
			glDetachShader(m_RendererID, vertexShader);
		if (!fragmentPath.empty())
			glDetachShader(m_RendererID, fragmentShader);
		if (!computeShaderPath.empty())
			glDetachShader(m_RendererID, computeShader);
	}

	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() {
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() {
		glUseProgram(0);
	}

	void OpenGLShader::SetUniform1f(const std::string& name, float value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::SetUniform2f(const std::string& name, const glm::vec2& vector) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2fv(location, 1, glm::value_ptr(vector));
	}

	void OpenGLShader::SetUniform3f(const std::string& name, const glm::vec3& vector) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3fv(location, 1, glm::value_ptr(vector));
	}

	void OpenGLShader::SetUniform4f(const std::string& name, const glm::vec4& vector) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4fv(location, 1, glm::value_ptr(vector));
	}

	void OpenGLShader::SetUniformMat3f(const std::string& name, const glm::mat3& matrix) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	GLuint OpenGLShader::CompileFragmentShader(const std::string& fragmentSrc) {
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const GLchar* fragmentSource = fragmentSrc.c_str();
		glShaderSource(fragmentShader, 1, &fragmentSource, 0);

		glCompileShader(fragmentShader);

		GLint isCompiled = 0;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(fragmentShader);
			LR_CORE_ERROR("Fragment shader compilation failed: {0}", infoLog.data());
			LR_ASSERT(false, "Fragment shader compilation failed!");

			return 0;
		}

		return fragmentShader;
	}

	GLuint OpenGLShader::CompileVertexShader(const std::string& vertexSrc) {
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		const GLchar* vertexSource = vertexSrc.c_str();
		glShaderSource(vertexShader, 1, &vertexSource, 0);

		glCompileShader(vertexShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(vertexShader);
			LR_CORE_ERROR("Vertex shader compilation failed: {0}", infoLog.data());
			LR_ASSERT(false, "Vertex shader compilation failed!");

			return 0;
		}

		return vertexShader;
	}

	GLuint OpenGLShader::CompileComputeShader(const std::string& computeSrc) {
		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

		const GLchar* computeSource = computeSrc.c_str();
		glShaderSource(computeShader, 1, &computeSource, 0);

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
			LR_ASSERT(false, "Compute shader compilation failed!");

			return 0;
		}

		return computeShader;
	}

}

#endif // LORE_PLATFORM_WINDOWS
