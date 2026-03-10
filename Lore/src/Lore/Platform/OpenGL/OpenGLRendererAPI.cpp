#include "lrpch.h"

#ifdef LORE_PLATFORM_WINDOWS

#include "Lore/Platform/OpenGL/OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Lore {

	OpenGLRendererAPI::~OpenGLRendererAPI() {
		if (m_BlitProgram) {
			glDeleteProgram(m_BlitProgram);
			m_BlitProgram = 0;
		}
		if (m_EmptyVAO) {
			glDeleteVertexArrays(1, &m_EmptyVAO);
			m_EmptyVAO = 0;
		}
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) {
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) {
		glDispatchCompute(groupX, groupY, groupZ);
	}

	void OpenGLRendererAPI::ComputeBarrier() {
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void OpenGLRendererAPI::InitBlitResources() {
		if (m_BlitProgram != 0) return;

		// Compile inline blit vertex shader — full-screen triangle from gl_VertexID
		const char* blitVertSrc = R"(
#version 460 core
out vec2 v_TexCoord;
void main() {
    v_TexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(v_TexCoord * 2.0 - 1.0, 0.0, 1.0);
}
)";

		const char* blitFragSrc = R"(
#version 460 core
in vec2 v_TexCoord;
out vec4 FragColor;
uniform sampler2D u_Texture;
void main() {
    FragColor = texture(u_Texture, v_TexCoord);
}
)";

		// Compile vertex shader
		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, &blitVertSrc, nullptr);
		glCompileShader(vertShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertShader, maxLength, &maxLength, &infoLog[0]);
			glDeleteShader(vertShader);
			LR_CORE_ERROR("Blit vertex shader compilation failed: {0}", infoLog.data());
			return;
		}

		// Compile fragment shader
		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &blitFragSrc, nullptr);
		glCompileShader(fragShader);

		isCompiled = 0;
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragShader, maxLength, &maxLength, &infoLog[0]);
			glDeleteShader(fragShader);
			glDeleteShader(vertShader);
			LR_CORE_ERROR("Blit fragment shader compilation failed: {0}", infoLog.data());
			return;
		}

		// Link program
		m_BlitProgram = glCreateProgram();
		glAttachShader(m_BlitProgram, vertShader);
		glAttachShader(m_BlitProgram, fragShader);
		glLinkProgram(m_BlitProgram);

		GLint isLinked = 0;
		glGetProgramiv(m_BlitProgram, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(m_BlitProgram, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_BlitProgram, maxLength, &maxLength, &infoLog[0]);
			glDeleteProgram(m_BlitProgram);
			m_BlitProgram = 0;
			LR_CORE_ERROR("Blit shader program linking failed: {0}", infoLog.data());
		}

		glDetachShader(m_BlitProgram, vertShader);
		glDetachShader(m_BlitProgram, fragShader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		// Create empty VAO for the full-screen triangle (no vertex buffers needed)
		glGenVertexArrays(1, &m_EmptyVAO);
	}

	void OpenGLRendererAPI::BlitToScreen(void* nativeTexture, uint32_t width, uint32_t height) {
		InitBlitResources();

		if (!m_BlitProgram || !m_EmptyVAO) return;

		glUseProgram(m_BlitProgram);

		// Bind the compute texture as a regular sampled texture
		GLuint texID = (GLuint)(uintptr_t)nativeTexture;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID);

		GLint loc = glGetUniformLocation(m_BlitProgram, "u_Texture");
		glUniform1i(loc, 0);

		// Draw full-screen triangle (3 vertices, no VBO)
		glBindVertexArray(m_EmptyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);
		glUseProgram(0);
	}

}

#endif // LORE_PLATFORM_WINDOWS
