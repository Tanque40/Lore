#pragma once

#include "RenderCommand.h"
#include "Shader.h"
#include "VertexArray.h"

namespace Lore {

    class Renderer {
    public:
        static void Init() {
            RenderCommand::Init();
        }

        static void BeginScene() {
            // TODO: Camera, lights, environment
        }

        static void EndScene() {
        }

        static void Submit(const Shader* shader, const VertexArray* vertexArray) {
            shader->Bind();
            vertexArray->Bind();
            RenderCommand::DrawIndexed(vertexArray);
        }

        static RendererAPIType GetAPI() { return RendererAPI::GetAPI(); }
    };

}
