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

					m_VoxelGrid = m_VoxelGrid.CastToVoxels(&m_Grid3D, 256);

					m_WorldSize = static_cast<float>(m_VoxelGrid.GetSize());
					m_MaxLevels = std::log2(m_WorldSize);

					m_SVOData = m_SVOBuilder.Build(m_VoxelGrid);
					m_VoxelGrid.Clear();

					uint32_t bufferSize = m_SVOData.size() * sizeof(SVO::SVONode);
					// Lo enlazamos al slot 0 (binding=0)
					svoBuffer.reset(Lore::StorageBuffer::Create(bufferSize, 0));

					svoBuffer->SetData(m_SVOData.data(), bufferSize);

					// Fuera del laberinto todo es piedra sólida: la cámara tiene que
					// arrancar DENTRO del primer pasillo tallado (la celda de entrada),
					// no en su posición libre anterior, o quedaría embebida en roca.
					Maze::Cell3D* entryCell = m_Grid3D(0, 0, 0);
					float entryIndexCenter = static_cast<float>(SVO::VoxelGrid::kCellsPerAxis) * 0.5f;
					glm::vec3 entryVoxelCenter = glm::vec3(entryIndexCenter) * m_VoxelScale;
					glm::vec3 lookDir = { 1.0f, 0.0f, 0.0f };
					if (entryCell->IsLinked(entryCell->GetEast())) {
						lookDir = { 1.0f, 0.0f, 0.0f };
					} else if (entryCell->IsLinked(entryCell->GetUp())) {
						lookDir = { 0.0f, 1.0f, 0.0f };
					} else if (entryCell->IsLinked(entryCell->GetSouth())) {
						lookDir = { 0.0f, 0.0f, 1.0f };
					}
					m_Camera.SetPosition(entryVoxelCenter);
					m_Camera.SetDirection(lookDir);
				}

				ImGui::TableNextColumn();
				if (ImGui::Button("Sidewinder 3D")) {
					m_Grid3D = Maze::Grid3D(m_Grid3DDimension, m_Grid3DDimension, m_Grid3DDimension);
					Maze::Sidewinder::On(&m_Grid3D);
					m_Grid3DString = m_Grid3D.ToString();

					m_VoxelGrid = m_VoxelGrid.CastToVoxels(&m_Grid3D, 256);

					m_WorldSize = static_cast<float>(m_VoxelGrid.GetSize());
					m_MaxLevels = std::log2(m_WorldSize);

					m_SVOData = m_SVOBuilder.Build(m_VoxelGrid);
					m_VoxelGrid.Clear();

					uint32_t bufferSize = m_SVOData.size() * sizeof(SVO::SVONode);
					// Lo enlazamos al slot 0 (binding=0)
					svoBuffer.reset(Lore::StorageBuffer::Create(bufferSize, 0));

					svoBuffer->SetData(m_SVOData.data(), bufferSize);

					// Misma razon que en Binary Tree 3D: la camara tiene que arrancar
					// dentro del primer pasillo tallado (la celda de entrada), no en
					// una posicion libre que podria caer en roca solida.
					Maze::Cell3D* entryCell = m_Grid3D(0, 0, 0);
					float entryIndexCenter = static_cast<float>(SVO::VoxelGrid::kCellsPerAxis) * 0.5f;
					glm::vec3 entryVoxelCenter = glm::vec3(entryIndexCenter) * m_VoxelScale;
					glm::vec3 lookDir = { 1.0f, 0.0f, 0.0f };
					if (entryCell->IsLinked(entryCell->GetEast())) {
						lookDir = { 1.0f, 0.0f, 0.0f };
					} else if (entryCell->IsLinked(entryCell->GetUp())) {
						lookDir = { 0.0f, 1.0f, 0.0f };
					} else if (entryCell->IsLinked(entryCell->GetSouth())) {
						lookDir = { 0.0f, 0.0f, 1.0f };
					}
					m_Camera.SetPosition(entryVoxelCenter);
					m_Camera.SetDirection(lookDir);
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
		ImGui::SliderFloat3("Camera Pos", m_Camera.GetPositionPtr(), -100.0f, 100.0f, "%.3f");
		ImGui::SliderFloat("Movement Speed", m_Camera.GetMovemetSpeedPtr(), 0.0f, 20.0f, "%.3f");
		ImGui::SliderFloat("Camera angle", m_Camera.GetFovPtr(), 0.0f, 90.0f, "%.3f");

		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("LOD")) {
		ImGui::BeginChild("LOD Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		ImGui::TextWrapped("El ray marcher deja de bajar por el octree cuando un nodo ya se ve mas chico en pantalla que este umbral, y lo dibuja como un bloque con el color promedio de su subarbol en vez de resolver el detalle exacto. Mas lejos = nodos mas grandes cortados antes = menos resolucion visible; mas cerca = se sigue bajando hasta el detalle fino.");
		ImGui::SliderFloat("Umbral LOD (tamano/distancia)", &m_LodPixelThreshold, 0.0f, 0.2f, "%.4f");
		ImGui::TextWrapped("0 desactiva el LOD (siempre se baja hasta el fondo del octree).");
		ImGui::SliderFloat("Cobertura minima solida", &m_LodCoverageThreshold, 0.0f, 1.0f, "%.2f");
		ImGui::TextWrapped("Que tan solido debe ser un nodo lejano para dibujarse opaco en vez de saltarse como aire. Bajarlo hace que estructuras finas/delgadas se sigan distinguiendo de lejos, a costa de infladas.");
		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("Voxel Grid")) {
		ImGui::BeginChild("Voxel Grid Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

		ImGui::SeparatorText("Estalactita de alta resolucion");
		ImGui::InputInt("Resolucion", (int*)&m_StalactiteResolution);
		ImGui::TextWrapped("Se redondea a la potencia de 2 mas cercana hacia arriba. Todo el indice se dedica a un unico objeto (en vez de repartirse sobre un mundo grande), asi el mismo costo de memoria produce voxeles mucho mas finos.");

		if (ImGui::Button("Generar Estalactita")) {
			m_VoxelGrid = SVO::VoxelGrid::CreateStalactite(m_StalactiteResolution);

			m_WorldSize = static_cast<float>(m_VoxelGrid.GetSize());
			m_MaxLevels = std::log2(m_WorldSize);
			// La estalactita es un objeto pequeño: encogemos voxelScale para que ocupe
			// siempre ~6 unidades de mundo sin importar la resolucion elegida, asi subir
			// la resolucion se traduce en voxeles mas chicos sobre el mismo objeto (no en
			// un mundo mas grande).
			m_VoxelScale = 6.0f / m_WorldSize;

			m_SVOData = m_SVOBuilder.Build(m_VoxelGrid);
			m_VoxelGridString = m_VoxelGrid.ToString();
			m_VoxelGrid.Clear();

			uint32_t bufferSize = static_cast<uint32_t>(m_SVOData.size() * sizeof(SVO::SVONode));
			svoBuffer.reset(Lore::StorageBuffer::Create(bufferSize, 0));
			svoBuffer->SetData(m_SVOData.data(), bufferSize);

			// El ray marcher del shader no recorta el rayo contra el AABB del mundo: si la
			// cámara arranca afuera de [0, worldSize] en cualquier eje, la primera
			// comprobación de límites corta el rayo antes de entrar y no se ve nada (aunque
			// el SVO se haya calculado bien). Por eso la cámara tiene que quedar DENTRO de
			// esos límites, no a una distancia proporcional a worldSize hacia afuera.
			float worldSize = m_WorldSize * m_VoxelScale;
			glm::vec3 lookTarget(worldSize * 0.5f, worldSize * 0.6f, worldSize * 0.5f);
			glm::vec3 cameraPos(worldSize * 0.15f, worldSize * 0.2f, worldSize * 0.15f);
			m_Camera.SetPosition(cameraPos);
			m_Camera.SetDirection(glm::normalize(lookTarget - cameraPos));
			*m_Camera.GetMovemetSpeedPtr() = 1.0f; // El objeto es chico: la velocidad por defecto lo sobrepasaria de inmediato
		}

		ImGui::Spacing();
		ImGui::SeparatorText("Estalagmita de alta resolucion");
		ImGui::InputInt("Resolucion##Stalagmite", (int*)&m_StalagmiteResolution);
		ImGui::TextWrapped("Mismos principios que la estalactita (mismo generador, invertido): crece del piso hacia arriba en vez de colgar del techo.");

		if (ImGui::Button("Generar Estalagmita")) {
			m_VoxelGrid = SVO::VoxelGrid::CreateStalagmite(m_StalagmiteResolution);

			m_WorldSize = static_cast<float>(m_VoxelGrid.GetSize());
			m_MaxLevels = std::log2(m_WorldSize);
			m_VoxelScale = 6.0f / m_WorldSize;

			m_SVOData = m_SVOBuilder.Build(m_VoxelGrid);
			m_VoxelGridString = m_VoxelGrid.ToString();
			m_VoxelGrid.Clear();

			uint32_t bufferSize = static_cast<uint32_t>(m_SVOData.size() * sizeof(SVO::SVONode));
			svoBuffer.reset(Lore::StorageBuffer::Create(bufferSize, 0));
			svoBuffer->SetData(m_SVOData.data(), bufferSize);

			// Misma cámara "dentro de los límites del mundo" que la estalactita (ver
			// comentario arriba): la geometría cambia de orientación pero el objeto sigue
			// ocupando aproximadamente la misma región del mundo local [0, worldSize].
			float worldSize = m_WorldSize * m_VoxelScale;
			glm::vec3 lookTarget(worldSize * 0.5f, worldSize * 0.6f, worldSize * 0.5f);
			glm::vec3 cameraPos(worldSize * 0.15f, worldSize * 0.2f, worldSize * 0.15f);
			m_Camera.SetPosition(cameraPos);
			m_Camera.SetDirection(glm::normalize(lookTarget - cameraPos));
			*m_Camera.GetMovemetSpeedPtr() = 1.0f;
		}

		double svoMegabytes = (m_SVOData.size() * sizeof(SVO::SVONode)) / (1024.0 * 1024.0);
		ImGui::Text("SVO: %zu nodos (%.2f MB en GPU)", m_SVOData.size(), svoMegabytes);

		ImGui::Separator();
		ImGui::Text("%s", m_VoxelGridString.c_str());

		ImGui::EndChild();
	}

	ImGui::End();
}

