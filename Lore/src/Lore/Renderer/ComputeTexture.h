#pragma once

#include <cstdint>
#include <memory>

namespace Lore {

	class ComputeTexture {
	public:
		virtual ~ComputeTexture() = default;

		/// Bind as a writable image for compute shaders (UAV / Storage Image).
		virtual void BindAsImage(uint32_t unit = 0) = 0;

		/// Bind as a readable texture for sampling in fragment shaders.
		virtual void BindAsTexture(uint32_t slot = 0) = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		/// Returns the platform-native handle (GLuint cast to void*, or id<MTLTexture>).
		virtual void* GetNativeHandle() const = 0;

		static ComputeTexture* Create(uint32_t width, uint32_t height);
	};

}
