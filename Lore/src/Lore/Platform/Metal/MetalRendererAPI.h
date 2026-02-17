#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/RendererAPI.h"

namespace Lore {

    class MetalRendererAPI : public RendererAPI {
    public:
        virtual void Init() override;
        virtual void SetClearColor(float r, float g, float b, float a) override;
        virtual void Clear() override;
        virtual void DrawIndexed(const VertexArray* vertexArray) override;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    };

}

#endif
