#include "lrpch.h"

#ifdef LORE_PLATFORM_WINDOWS

#include "Lore/Platform/OpenGL/OpenGLComputeShader.h"

#include <glm/gtc/type_ptr.hpp>

namespace Lore {

	namespace {

		bool CheckShader(GLuint shader, const std::string& what) {
			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_TRUE)
				return true;

			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(std::max(maxLength, 1), '\0');
			glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());
			LR_CORE_ERROR("{0} falló: {1}", what, infoLog.data());
			return false;
		}

		// SPIR-V con GL_ARB_gl_spirv (core en 4.6): el driver no vuelve a compilar GLSL, sólo
		// especializa el binario que generó glslangValidator en el build.
		GLuint CreateFromSpirv(const std::string& binary, const std::string& path) {
			GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
			glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, binary.data(), static_cast<GLsizei>(binary.size()));
			glSpecializeShaderARB(shader, "main", 0, nullptr, nullptr);
			if (!CheckShader(shader, "Especializar SPIR-V " + path)) {
				glDeleteShader(shader);
				return 0;
			}
			return shader;
		}

		GLuint CreateFromGlsl(const std::string& source, const std::string& path) {
			GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
			const GLchar* src = source.c_str();
			glShaderSource(shader, 1, &src, 0);
			glCompileShader(shader);
			if (!CheckShader(shader, "Compilar GLSL " + path)) {
				glDeleteShader(shader);
				return 0;
			}
			return shader;
		}

	}

	OpenGLComputeShader::OpenGLComputeShader(const std::string& computePath) {
		// computePath es la ruta base del shader compilado (sin extensión). Preferimos el
		// SPIR-V; si el driver no expone GL_ARB_gl_spirv (o lo rechaza) usamos el GLSL que
		// spirv-cross regenera del mismo SPIR-V, así que la lógica es la misma en ambos casos.
		GLuint computeShader = 0;
		std::string spirvPath = computePath + ".gl.spv";
		std::string glslPath = computePath + ".gl.glsl";

		if (GLAD_GL_ARB_gl_spirv) {
			std::string binary = ReadFile(spirvPath);
			if (!binary.empty())
				computeShader = CreateFromSpirv(binary, spirvPath);
		} else {
			LR_CORE_WARN("El driver no soporta GL_ARB_gl_spirv; usando {0}", glslPath);
		}

		if (!computeShader) {
			std::string source = ReadFile(glslPath);
			if (!source.empty())
				computeShader = CreateFromGlsl(source, glslPath);
		}

		if (!computeShader) {
			LR_CORE_ASSERT(false, "No se pudo cargar el compute shader (¿corriste Scripts/CompileShaders.py?)");
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

			std::vector<GLchar> infoLog(std::max(maxLength, 1), '\0');
			glGetProgramInfoLog(m_ProgramID, maxLength, &maxLength, infoLog.data());

			glDeleteProgram(m_ProgramID);
			glDeleteShader(computeShader);
			m_ProgramID = 0;

			LR_CORE_ERROR("Compute shader program linking failed: {0}", infoLog.data());
			LR_CORE_ASSERT(false, "Compute shader program linking failed!");
			return;
		}

		glDetachShader(m_ProgramID, computeShader);
		glDeleteShader(computeShader);
		LR_CORE_INFO("Compute shader cargado: {0}", computePath);
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
