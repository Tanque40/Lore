#pragma once

#include "RendererAPI.h"
#include "VertexArray.h"

namespace Lore {

    class RenderCommand {
    public:
        static void Init() {
            s_RendererAPI->Init();
        }

        static void SetClearColor(float r, float g, float b, float a) {
            s_RendererAPI->SetClearColor(r, g, b, a);
        }

        static void Clear() {
            s_RendererAPI->Clear();
        }

        static void DrawIndexed(const VertexArray* vertexArray) {
            s_RendererAPI->DrawIndexed(vertexArray);
        }

        static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
            s_RendererAPI->SetViewport(x, y, width, height);
        }

        static RendererAPI* GetRendererAPI() { return s_RendererAPI; }

    private:
        static RendererAPI* s_RendererAPI;
    };

}
