#include "lrpch.h"
#include "OpenGLShader.h"

#include <glad/glad.h>

namespace Lore {

	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc) {

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
			LR_CORE_ASSERT(false, "Vertex shader compilation failed!");

			return;
		}

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const GLchar* fragmentSource = fragmentSrc.c_str();
		glShaderSource(fragmentShader, 1, &fragmentSource, 0);

		glCompileShader(fragmentShader);

		isCompiled = 0;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(fragmentShader);
			LR_CORE_ERROR("Fragment shader compilation failed: {0}", infoLog.data());
			LR_CORE_ASSERT(false, "Fragment shader compilation failed!");

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
			LR_CORE_ASSERT(false, "Shader program linking failed!");

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			return;
		}

		glDetachShader(m_RendererID, vertexShader);
		glDetachShader(m_RendererID, fragmentShader);
	}

	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const {
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const {
		glUseProgram(0);
	}

}
