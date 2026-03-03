#pragma once

#include "Lore/Renderer/VertexArray.h"

#ifdef __OBJC__
#import <Metal/Metal.h>
#endif

namespace Lore {

	class MetalVertexArray : public VertexArray {
	private:
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;

	public:
		MetalVertexArray();
		~MetalVertexArray() = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		inline virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		inline virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
	};

}
