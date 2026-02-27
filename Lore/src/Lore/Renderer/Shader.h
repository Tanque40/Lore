#pragma once

#include <glm/glm.hpp>

namespace Lore {

	class Shader {
	private:
		unsigned int m_RendererID;
	public:
		Shader(const std::string& vertexPath, const std::string& fragmentPath);
		~Shader();

		void Bind();
		void Unbind();

		void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
	};

}
