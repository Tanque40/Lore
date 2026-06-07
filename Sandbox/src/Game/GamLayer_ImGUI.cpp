#include "sndbxpch.h"

#include "Game/GameLayer.h"

#include "Maze/Algorithms/BinaryTree.h"
#include "Maze/Algorithms/Wilsons.h"
#include "Maze/Algorithms/Sidewinder.h"

void GameLayer::OnImGuiRender() {
	Lore::Application& app = Lore::Application::Get();
	Lore::FramebufferSpecification spec = app.GetFramebuffer().GetSpecification();
	ImGui::Begin("Compute-First Demo");

	if (!ImGui::CollapsingHeader("Stats")) {
		ImGui::BeginChild("Stats Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		ImGui::Text("Resolution: %dx%d", m_Width, m_Height);
		ImGui::Text("Framebuffer Size: %dx%d", spec.Width, spec.Height);
		ImGui::Text("FPS: %.2f (%.2f ms)", m_FPS, m_FrameTime * 1000.0f);
		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("Maze")) {
		ImGui::BeginChild("Maze Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		{
			ImGui::SeparatorText("Maze Generation");
			ImGui::InputInt("Grid Dimension", (int*)&m_GridDimension);

			if (ImGui::BeginTable("Maze2DButtonsTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {

				ImGui::TableNextColumn();
				if (ImGui::Button("Binary Tree")) {
					m_Grid = Maze::Grid(m_GridDimension, m_GridDimension);
					Maze::Grid grid = Maze::BinaryTree::On(m_Grid);
					m_GridString = grid.ToString();
				}

				ImGui::TableNextColumn();
				if (ImGui::Button("Wilson's")) {
					m_Grid = Maze::Grid(m_GridDimension, m_GridDimension);
					Maze::Grid grid = Maze::Wilsons::On(m_Grid);
					m_GridString = grid.ToString();
				}

				ImGui::TableNextColumn();
				if (ImGui::Button("Sidewinder")) {
					m_Grid = Maze::Grid(m_GridDimension, m_GridDimension);
					Maze::Grid grid = Maze::Sidewinder::On(m_Grid);
					m_GridString = grid.ToString();
				}

				ImGui::EndTable();
			}

			{
				int maxHeightInLines = 20;
				ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(FLT_MAX, ImGui::GetTextLineHeightWithSpacing() * maxHeightInLines));
				ImGui::BeginChild("Show Maze", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::Text("%s", m_GridString.c_str());
				ImGui::EndChild();
			}

			ImGui::InputInt("Grid 3D size", (int*)&m_Grid3DDimension);

			if (ImGui::BeginTable("Maze3DButtonsTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {

				ImGui::TableNextColumn();
				if (ImGui::Button("Binary Tree 3D")) {
					m_Grid3D = Maze::Grid3D(m_Grid3DDimension, m_Grid3DDimension, m_Grid3DDimension);
					Maze::BinaryTree::On(&m_Grid3D);
					m_Grid3DString = m_Grid3D.ToString();

					m_VoxelGrid = m_VoxelGrid.CastToVoxels(&m_Grid3D, 128);
					m_SVOData = m_SVOBuilder.Build(m_VoxelGrid);

					uint32_t bufferSize = m_SVOData.size() * sizeof(SVO::SVONode);
					// Lo enlazamos al slot 0 (binding=0)
					svoBuffer.reset(Lore::StorageBuffer::Create(bufferSize, 0));

					// 3. Subes los datos a la GPU
					svoBuffer->SetData(m_SVOData.data(), bufferSize);

				}

				ImGui::EndTable();
			}

			{
				int maxHeightInLines = 15;
				ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(FLT_MAX, ImGui::GetTextLineHeightWithSpacing() * maxHeightInLines));
				ImGui::BeginChild("Show Maze 3D", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::Text("%s", m_Grid3DString.c_str());
				ImGui::EndChild();
			}

			{
				int maxHeightInLines = 15;
				ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(FLT_MAX, ImGui::GetTextLineHeightWithSpacing() * maxHeightInLines));
				ImGui::BeginChild("Show Maze 3D Int Matrix", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::Text("%s", m_Grid3DIntMatrixString.c_str());
				ImGui::EndChild();
			}
		}

		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("Camera")) {
		ImGui::BeginChild("Camera Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		ImGui::SliderFloat("Movement Speed", m_Camera.GetMovemetSpeedPtr(), 0.0f, 20.0f, "%.3f");
		ImGui::SliderFloat("Camera angle", m_Camera.GetFovPtr(), 0.0f, 90.0f, "%.3f");

		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("Voxel Grid")) {
		ImGui::BeginChild("Voxel Grid Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		ImGui::Text("%s", m_VoxelGridString.c_str());

		ImGui::EndChild();
	}

	ImGui::End();
}

