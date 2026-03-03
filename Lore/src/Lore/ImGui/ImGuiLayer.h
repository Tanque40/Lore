#pragma once

#include "Lore/Layer.h"

#include "Lore/Events/ApplicationEvent.h"
#include "Lore/Events/KeyEvent.h"
#include "Lore/Events/MouseEvent.h"

namespace Lore {

	class  ImGuiLayer : public Layer {
	private:
		float m_Time = 0.0f;
		bool m_DockspaceInitialized = false;

	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

	private:
		void BeginDockspace();
		void EndDockspace();
		void RenderViewport();
	};

}

