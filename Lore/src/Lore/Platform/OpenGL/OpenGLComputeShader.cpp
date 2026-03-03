#include "lrpch.h"

#include "Lore/Platform/OpenGL/OpenGLComputeShader.h"

namespace Lore {

    OpenGLComputeShader::OpenGLComputeShader(const std::string& computePath) {
        std::string source = ReadFile(computePath);

        // Compile the compute shader
        GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
        const GLchar* src = source.c_str();
        glShaderSource(computeShader, 1, &src, 0);
        glCompileShader(computeShader);

        GLint isCompiled = 0;
        glGetShaderiv(computeShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(computeShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(computeShader);
            LR_CORE_ERROR("Compute shader compilation failed: {0}", infoLog.data());
            LR_CORE_ASSERT(false, "Compute shader compilation failed!");
            return;
        }

        // Create a separate program for the compute shader
        // (OpenGL requires compute shaders to be in their own program)
        m_ProgramID = glCreateProgram();
        glAttachShader(m_ProgramID, computeShader);
        glLinkProgram(m_ProgramID);

        GLint isLinked = 0;
        glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(m_ProgramID, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(m_ProgramID, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(m_ProgramID);
            glDeleteShader(computeShader);
            m_ProgramID = 0;

            LR_CORE_ERROR("Compute shader program linking failed: {0}", infoLog.data());
            LR_CORE_ASSERT(false, "Compute shader program linking failed!");
            return;
        }

        glDetachShader(m_ProgramID, computeShader);
        glDeleteShader(computeShader);
    }

    OpenGLComputeShader::~OpenGLComputeShader() {
        if (m_ProgramID) {
            glDeleteProgram(m_ProgramID);
        }
    }

    void OpenGLComputeShader::Bind() {
        glUseProgram(m_ProgramID);
    }

    void OpenGLComputeShader::Unbind() {
        glUseProgram(0);
    }

}
