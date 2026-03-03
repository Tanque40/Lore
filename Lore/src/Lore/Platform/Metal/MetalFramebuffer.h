#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/Framebuffer.h"

namespace Lore {

	class MetalFramebuffer : public Framebuffer {
	public:
		MetalFramebuffer(const FramebufferSpecification& spec);
		virtual ~MetalFramebuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void* GetColorAttachmentRendererID() const override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		void Invalidate();

		void* m_Texture = nullptr;  // id<MTLTexture>
		FramebufferSpecification m_Specification;
	};

}

#endif // LORE_PLATFORM_MAC
