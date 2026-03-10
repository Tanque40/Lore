#include "lrpch.h"

#ifdef LORE_PLATFORM_WINDOWS

#include "Lore/Platform/OpenGL/OpenGLComputeTexture.h"

namespace Lore {

	OpenGLComputeTexture::OpenGLComputeTexture(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height) {
		Invalidate();
	}

	OpenGLComputeTexture::~OpenGLComputeTexture() {
		if (m_TextureID) {
			glDeleteTextures(1, &m_TextureID);
		}
	}

	void OpenGLComputeTexture::Invalidate() {
		if (m_TextureID) {
			glDeleteTextures(1, &m_TextureID);
			m_TextureID = 0;
		}

		glGenTextures(1, &m_TextureID);
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_Width, m_Height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLComputeTexture::BindAsImage(uint32_t unit) {
		glBindImageTexture(unit, m_TextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	}

	void OpenGLComputeTexture::BindAsTexture(uint32_t slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
	}

	void OpenGLComputeTexture::Resize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0 || width > 8192 || height > 8192) {
			LR_CORE_WARN("Attempted to resize compute texture to {0}, {1}", width, height);
			return;
		}

		m_Width = width;
		m_Height = height;
		Invalidate();
	}

}

#endif // LORE_PLATFORM_WINDOWS
