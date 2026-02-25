#include <Lore.h>

#include <imgui.h>
#include <string>

#include "Maze/Algorithms/BinaryTree.h"

class ExampleLayer : public Lore::Layer {
private:
	Maze::Grid m_Grid{ 10, 10 };
	std::string m_GridString = "";

public:
	ExampleLayer() : Layer("Example") {

	}

	void OnAttach() override {
		m_Grid = Maze::BinaryTree::On(m_Grid);
		m_GridString = m_Grid.ToString();
		//LR_TRACE("\n{}", m_GridString);
	}

	void OnUpdate() override {
		if (Lore::Input::IsKeyPressed(LR_KEY_TAB)) {
			LR_TRACE("Tab key is pressed! (poll)");
		}
	}

	void OnImGuiRender() override {
		ImGui::Begin("SandBox Test");
		ImGui::Text("%s", m_GridString.c_str());
		ImGui::End();
	}

	void OnEvent(Lore::Event& event) override {
		if (event.GetEventType() == Lore::EventType::KeyPressed) {
			Lore::KeyPressedEvent& e = (Lore::KeyPressedEvent&)event;
			if (e.GetKeyCode() == LR_KEY_TAB)
				LR_TRACE("Tab key is pressed! (event)");
			LR_TRACE("{0}", (char)e.GetKeyCode());
		}
	}
};

class Sandbox : public Lore::Application {

public:
	Sandbox() {
		PushLayer(new ExampleLayer());
	}
	~Sandbox() {}

};


Lore::Application* Lore::CreateApplication() {
	return new Sandbox();
}


