#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/Buffer.h"

namespace Lore {

	class MetalVertexBuffer : public VertexBuffer {
	public:
		MetalVertexBuffer(float* vertices, uint32_t size);
		virtual ~MetalVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		virtual const BufferLayout& GetLayout() const override { return m_Layout; }

		void* GetMetalBuffer() const { return m_Buffer; }

	private:
		void* m_Buffer = nullptr;   // id<MTLBuffer>
		BufferLayout m_Layout;
	};

	class MetalIndexBuffer : public IndexBuffer {
	public:
		MetalIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~MetalIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }

		void* GetMetalBuffer() const { return m_Buffer; }

	private:
		void* m_Buffer = nullptr;   // id<MTLBuffer>
		uint32_t m_Count;
	};

}

#endif
