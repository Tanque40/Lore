#pragma once

namespace Lore {

	class Shader {
	private:
		unsigned int m_RendererID;
	public:
		Shader(const std::string& vertexPath, const std::string& fragmentPath);
		~Shader();

		void Bind();
		void Unbind();
	};

}
