#pragma once

#include "Lore/Renderer/ComputeTexture.h"

#include <glad/glad.h>

namespace Lore {

	class OpenGLComputeTexture : public ComputeTexture {
	public:
		OpenGLComputeTexture(uint32_t width, uint32_t height);
		virtual ~OpenGLComputeTexture();

		virtual void BindAsImage(uint32_t unit = 0) override;
		virtual void BindAsTexture(uint32_t slot = 0) override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void* GetNativeHandle() const override { return (void*)(uintptr_t)m_TextureID; }

	private:
		void Invalidate();

		GLuint m_TextureID = 0;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};

}
