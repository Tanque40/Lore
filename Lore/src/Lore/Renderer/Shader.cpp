#include "lrpch.h"
#include "Shader.h"

#include <glad/glad.h>

namespace Lore {

	Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		const GLchar* vertexSource = vertexPath.c_str();
		glShaderSource(vertexShader, 1, &vertexSource, 0);

		glCompileShader(vertexShader);

		GLint isCompuiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompuiled);
		if (isCompuiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(vertexShader);
			LR_CORE_ERROR("Vertex shader compilation failed: {0}", infoLog.data());
			LR_ASSERT(false, "Vertex shader compilation failed!");

			return;
		}


		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const GLchar* fragmentSource = fragmentPath.c_str();
		glShaderSource(fragmentShader, 1, &fragmentSource, 0);

		glCompileShader(fragmentShader);

		isCompuiled = 0;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompuiled);
		if (isCompuiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(fragmentShader);
			LR_CORE_ERROR("Fragment shader compilation failed: {0}", infoLog.data());
			LR_ASSERT(false, "Fragment shader compilation failed!");

			return;
		}

		m_RendererID = glCreateProgram();

		glAttachShader(m_RendererID, vertexShader);
		glAttachShader(m_RendererID, fragmentShader);

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

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			return;
		}

		glDetachShader(m_RendererID, vertexShader);
		glDetachShader(m_RendererID, fragmentShader);

	}

	Shader::~Shader() {
		glDeleteProgram(m_RendererID);
	}

	void Shader::Bind() {
		glUseProgram(m_RendererID);
	}

	void Shader::Unbind() {
		glUseProgram(0);
	}

}
