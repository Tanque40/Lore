#pragma once

#include <string>
#include <memory>

namespace Lore {

	class ComputeShader {
	public:
		virtual ~ComputeShader() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		static ComputeShader* Create(const std::string& computePath);

	protected:
		std::string ReadFile(const std::string& filePath);
	};

}
