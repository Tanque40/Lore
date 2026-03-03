#pragma once

#include "Lore/Renderer/ComputeShader.h"

#include <glad/glad.h>

namespace Lore {

    class OpenGLComputeShader : public ComputeShader {
    public:
        OpenGLComputeShader(const std::string& computePath);
        virtual ~OpenGLComputeShader();

        virtual void Bind() override;
        virtual void Unbind() override;

    private:
        GLuint m_ProgramID = 0;
    };

}
