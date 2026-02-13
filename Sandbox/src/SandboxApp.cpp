#include <Lore.h>

#include <imgui.h>

class ExampleLayer : public Lore::Layer {
public:
	ExampleLayer() : Layer("Example") {
	}

	void OnUpdate() override {
		if (Lore::Input::IsKeyPressed(LR_KEY_TAB)) {
			LR_TRACE("Tab key is pressed! (poll)");
		}
	}

	void OnImGuiRender() override {
		ImGui::Begin("SandBox Test");
		ImGui::Text("Hello, World!");
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


