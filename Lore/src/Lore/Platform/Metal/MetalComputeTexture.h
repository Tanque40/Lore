#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/ComputeTexture.h"

namespace Lore {

	class MetalComputeTexture : public ComputeTexture {
	public:
		MetalComputeTexture(uint32_t width, uint32_t height);
		virtual ~MetalComputeTexture();

		virtual void BindAsImage(uint32_t unit = 0) override;
		virtual void BindAsTexture(uint32_t slot = 0) override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void* GetNativeHandle() const override { return m_Texture; }

	private:
		void Invalidate();

		void* m_Texture = nullptr;  // id<MTLTexture>
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};

}

#endif // LORE_PLATFORM_MAC
