#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/VertexArray.h"

namespace Lore {

    class MetalVertexArray : public VertexArray {
    public:
        MetalVertexArray();
        virtual ~MetalVertexArray();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void AddVertexBuffer(VertexBuffer* vertexBuffer) override;
        virtual void SetIndexBuffer(IndexBuffer* indexBuffer) override;

        virtual const std::vector<VertexBuffer*>& GetVertexBuffers() const override { return m_VertexBuffers; }
        virtual const IndexBuffer* GetIndexBuffer() const override { return m_IndexBuffer; }

    private:
        std::vector<VertexBuffer*> m_VertexBuffers;
        IndexBuffer* m_IndexBuffer = nullptr;
    };

}

#endif
