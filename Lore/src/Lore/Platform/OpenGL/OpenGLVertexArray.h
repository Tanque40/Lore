#pragma once

#include "Lore/Renderer/VertexArray.h"

namespace Lore {

	class OpenGLVertexArray : public VertexArray {
	private:
		uint32_t m_RendererID;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;

	public:
		OpenGLVertexArray();
		~OpenGLVertexArray() = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
	};

}
