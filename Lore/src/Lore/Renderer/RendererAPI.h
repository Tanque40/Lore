#pragma once

namespace Lore {

	class VertexArray;

	enum class RendererAPIType {
		None = 0,
		OpenGL = 1,
		Metal = 2
	};

	class RendererAPI {
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear() = 0;
		virtual void DrawIndexed(const VertexArray* vertexArray) = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		static RendererAPIType GetAPI() { return s_API; }
		static void SetAPI(RendererAPIType api) { s_API = api; }

		static RendererAPI* Create();

	private:
		static RendererAPIType s_API;
	};

}
