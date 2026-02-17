#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/Shader.h"
#include "Lore/Renderer/Buffer.h"

namespace Lore {

    class MetalShader : public Shader {
    public:
        MetalShader(const std::string& vertexSrc, const std::string& fragmentSrc);
        virtual ~MetalShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        void* GetPipelineState() const { return m_PipelineState; }

    private:
        void CreatePipelineState(const std::string& vertexSrc, const std::string& fragmentSrc);

        void* m_PipelineState = nullptr;   // id<MTLRenderPipelineState>
        void* m_Library = nullptr;         // id<MTLLibrary>
    };

}

#endif
