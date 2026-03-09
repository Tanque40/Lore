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

		// ? Floats
		virtual void SetUniform1f(const std::string& name, float value) override;
		virtual void SetUniform2f(const std::string& name, const glm::vec2& vector) override;
		virtual void SetUniform3f(const std::string& name, const glm::vec3& vector) override;
		virtual void SetUniform4f(const std::string& name, const glm::vec4& vector) override;

		// ? Matrices
		virtual void SetUniformMat3f(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) override;

    private:
        GLuint m_ProgramID = 0;
    };

}
