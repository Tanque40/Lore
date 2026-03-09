#pragma once

#include "Lore/Renderer/Shader.h"	

namespace Lore {

	class ComputeShader : public Shader{
	public:
		virtual ~ComputeShader() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		static ComputeShader* Create(const std::string& computePath);

	protected:
		std::string ReadFile(const std::string& filePath);
	};

}
