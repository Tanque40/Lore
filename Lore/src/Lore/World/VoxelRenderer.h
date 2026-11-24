#pragma once

#include "Lore/World/SVO.h"
#include "Lore/World/VoxelShaderTypes.h"

#include "Lore/Renderer/ComputeShader.h"
#include "Lore/Renderer/ComputeTexture.h"
#include "Lore/Renderer/Buffer.h"
#include "Lore/Renderer/PerspectiveCamera.h"

namespace Lore {

	struct VoxelRenderSettings {
		// Unidades de mundo por vóxel de índice. Bajarlo hace que los vóxeles se vean
		// más pequeños/finos sin cambiar cuántos hay ni el tamaño del mundo en índices.
		float VoxelScale = 0.25f;

		// LOD por distancia (ver TraceRay en los shaders): a qué fracción tamaño/distancia se
		// deja de bajar por el octree y se renderiza el nodo como un bloque aproximado en vez
		// de resolver el detalle exacto. 0 desactiva el LOD (siempre baja hasta el fondo).
		float LodPixelThreshold = 0.02f;
		// Cobertura mínima (0-1) de sólido en el subárbol de un nodo para que, al cortar el
		// descenso por LOD, se dibuje como bloque opaco en vez de tratarse como aire. Bajo a
		// propósito: geometría delgada (cáscaras, paredes finas) tiene poca cobertura en un
		// bloque grande, y un umbral alto (probado con 0.3) la hace desaparecer por completo
		// en vez de verse como un bloque aproximado.
		float LodCoverageThreshold = 0.05f;
	};

	// Ray marcher del SVO en compute shader (lo que antes vivía dentro de GameLayer): dueño
	// del shader, de la textura de salida y del buffer de nodos en la GPU. Sólo se usa desde
	// el hilo principal.
	class VoxelRenderer {
	private:
		std::shared_ptr<ComputeShader> m_ComputeShader;
		std::shared_ptr<ComputeTexture> m_ComputeTexture;
		std::shared_ptr<StorageBuffer> m_SVOBuffer;
		std::shared_ptr<UniformBuffer> m_FrameBuffer; // GPU::FrameData, binding LORE_BINDING_FRAME_DATA
		size_t m_NodeCount = 0;

		// Tamaño real y profundidad del octree actualmente subido a la GPU (el shader
		// necesita esto para saber cuántos niveles bajar al recorrer el SVO).
		float m_WorldSize = 0.0f;
		float m_MaxLevels = 0.0f;

		// Fracción del framebuffer real (en píxeles) que traza el compute shader. Por
		// defecto puntos/píxeles (0.5 en Retina) para no cuadruplicar el costo.
		float m_RenderScale = 1.0f;
		uint32_t m_Width = 1;
		uint32_t m_Height = 1;

	public:
		// computeShaderPath: ruta base del shader compilado, sin extensión (cada backend
		// agrega la suya: .metal en Mac, .gl.spv / .gl.glsl en OpenGL).
		explicit VoxelRenderer(const std::string& computeShaderPath);

		// Sube el octree a la GPU reemplazando el anterior. worldSize = lado del grid en
		// vóxeles (potencia de 2).
		void Upload(const std::vector<SVO::SVONode>& nodes, uint32_t worldSize);

		// Traza la escena con la cámara dada y la copia al framebuffer actual.
		void Render(const PerspectiveCamera& camera, const VoxelRenderSettings& settings);

		// Ajusta la textura de salida al framebuffer real * render scale. Llamar al cambiar
		// el tamaño de la ventana.
		void Resize();

		void SetRenderScale(float scale);
		float GetRenderScale() const { return m_RenderScale; }

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		size_t GetNodeCount() const { return m_NodeCount; }
		float GetWorldSize() const { return m_WorldSize; }
	};

}
