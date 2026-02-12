#pragma once

#include "Lore/Layer.h"

namespace Lore {

	class LORE_API ImGuiLayer : public Layer {
	private:
		float m_Time = 0.0f;

	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& e) override;
	};

}

