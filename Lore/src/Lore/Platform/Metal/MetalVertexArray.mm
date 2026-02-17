#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalVertexArray.h"
#include "MetalBuffer.h"
#include "MetalContext.h"

namespace Lore {

	MetalVertexArray::MetalVertexArray() {
		// Metal doesn't have VAOs - this is just a container
	}

	MetalVertexArray::~MetalVertexArray() {
		// Buffers are owned externally, we don't delete them here
	}

	void MetalVertexArray::Bind() const {
		// In Metal, "binding" a vertex array means setting all its vertex buffers on the encoder
		for (const auto* vb : m_VertexBuffers) {
			vb->Bind();
		}
	}

	void MetalVertexArray::Unbind() const {
		// Metal doesn't have a concept of unbinding
	}

	void MetalVertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer) {
		LR_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex buffer has no layout!");
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void MetalVertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {
		m_IndexBuffer = indexBuffer;
	}

}

#endif
