#pragma once

#ifdef __APPLE__
#include <cstdint>

#include "Lore/Renderer/Buffer.h"

namespace Lore {

	class MetalVertexBuffer : public VertexBuffer {
	private:
		void* m_Buffer = nullptr;   // id<MTLBuffer>
		BufferLayout m_Layout;

	public:
		MetalVertexBuffer(float* vertices, uint32_t size);
		virtual ~MetalVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		inline virtual const BufferLayout& GetLayout() const override { return m_Layout; }

		inline void* GetMetalBuffer() const { return m_Buffer; }
	};

	class MetalIndexBuffer : public IndexBuffer {
	private:
		void* m_Buffer = nullptr;   // id<MTLBuffer>
		uint32_t m_Count;

	public:
		MetalIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~MetalIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual uint32_t GetCount() const override { return m_Count; }

		inline virtual void* GetNativeHandle() const override { return m_Buffer; }
	};

}

#endif // LORE_PLATFORM_MAC
