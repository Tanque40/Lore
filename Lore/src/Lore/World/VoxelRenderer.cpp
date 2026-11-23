#include "lrpch.h"

#include "Lore/World/VoxelRenderer.h"

#include "Lore/Application.h"
#include "Lore/Renderer/RenderCommand.h"
#include "Lore/Renderer/Renderer.h"

#include <glm/glm.hpp>

namespace Lore {

	VoxelRenderer::VoxelRenderer(const std::string& computeShaderPath) {
		m_ComputeShader.reset(ComputeShader::Create(computeShaderPath));
		m_ComputeTexture.reset(ComputeTexture::Create(1, 1));

		// Por defecto trazamos tantos píxeles como puntos tiene la ventana: en Retina eso es
		// 0.5, en una pantalla 1x es 1.0. Subir a 1.0 en Retina da nitidez completa pero 4x
		// más rayos.
		Window& window = Application::Get().GetWindow();
		if (window.GetFramebufferWidth() > 0)
			m_RenderScale = std::clamp(static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetFramebufferWidth()), 0.25f, 1.0f);
		Resize();
	}

	void VoxelRenderer::Upload(const std::vector<SVO::SVONode>& nodes, uint32_t worldSize) {
		m_NodeCount = nodes.size();
		m_WorldSize = static_cast<float>(worldSize);
		m_MaxLevels = worldSize > 0 ? std::log2(m_WorldSize) : 0.0f;

		// Un SVO vacío (todo aire) igual necesita la raíz para que el shader lea algo válido.
		static const SVO::SVONode kEmptyRoot = { 0, 0, 0 };
		const void* data = nodes.empty() ? &kEmptyRoot : nodes.data();
		uint32_t bufferSize = static_cast<uint32_t>(std::max<size_t>(nodes.size(), 1) * sizeof(SVO::SVONode));

		// Lo enlazamos al slot 0 (binding=0)
		m_SVOBuffer.reset(StorageBuffer::Create(bufferSize, 0));
		m_SVOBuffer->SetData(data, bufferSize);
	}

	void VoxelRenderer::Render(const PerspectiveCamera& camera, const VoxelRenderSettings& settings) {
		if (!m_SVOBuffer)
			return;

		m_SVOBuffer->Bind(0);

		// 1. Bind compute resources
		m_ComputeShader->Bind();
		m_ComputeTexture->BindAsImage(0);

		m_ComputeShader->SetUniform3f("u_CameraPos", camera.GetPosition());
		m_ComputeShader->SetUniform3f("u_CameraDir", camera.GetDirection());
		m_ComputeShader->SetUniform3f("u_CameraUp", camera.GetUp());
		m_ComputeShader->SetUniform3f("u_CameraRight", camera.GetRight());
		m_ComputeShader->SetUniform1f("u_Fov", glm::radians(camera.GetFov()));
		m_ComputeShader->SetUniform1f("u_WorldSize", m_WorldSize);
		m_ComputeShader->SetUniform1f("u_MaxLevels", m_MaxLevels);
		m_ComputeShader->SetUniform1f("u_VoxelScale", settings.VoxelScale);
		m_ComputeShader->SetUniform1f("u_LodPixelThreshold", settings.LodPixelThreshold);
		m_ComputeShader->SetUniform1f("u_LodCoverageThreshold", settings.LodCoverageThreshold);

		// 2. Dispatch compute shader
		uint32_t groupsX = (m_Width + 15) / 16;
		uint32_t groupsY = (m_Height + 15) / 16;
		RenderCommand::DispatchCompute(groupsX, groupsY, 1);

		// 3. Memory barrier (ensures compute writes are visible)
		RenderCommand::ComputeBarrier();

		// 4. Clear and blit the compute texture to screen
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		RenderCommand::Clear();
		Renderer::BlitToScreen(m_ComputeTexture);
	}

	void VoxelRenderer::SetRenderScale(float scale) {
		m_RenderScale = std::clamp(scale, 0.1f, 1.0f);
		Resize();
	}

	void VoxelRenderer::Resize() {
		Window& window = Application::Get().GetWindow();
		uint32_t framebufferWidth = window.GetFramebufferWidth();
		uint32_t framebufferHeight = window.GetFramebufferHeight();
		if (framebufferWidth == 0 || framebufferHeight == 0) {
			// Ventana minimizada o tamaño de framebuffer aún no disponible.
			framebufferWidth = window.GetWidth();
			framebufferHeight = window.GetHeight();
		}

		float scale = std::clamp(m_RenderScale, 0.1f, 1.0f);
		m_Width = std::max(1u, static_cast<uint32_t>(framebufferWidth * scale));
		m_Height = std::max(1u, static_cast<uint32_t>(framebufferHeight * scale));
		m_ComputeTexture->Resize(m_Width, m_Height);
	}

}
