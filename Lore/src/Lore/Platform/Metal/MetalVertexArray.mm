#include "lrpch.h"

#include "MetalVertexArray.h"

#import <Metal/Metal.h>

namespace Lore {

	MetalVertexArray::MetalVertexArray() {
		// Metal doesn't have a VAO equivalent, but we track the buffers
	}

	void MetalVertexArray::Bind() const {
		// In Metal, vertex buffers are bound directly to the render encoder
		// This would be called before draw commands
		// The actual encoder binding happens in MetalRendererAPI::DrawIndexed
	}

	void MetalVertexArray::Unbind() const {
		// No-op for Metal
	}

	void MetalVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
		LR_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout");
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void MetalVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
		m_IndexBuffer = indexBuffer;
	}

}
