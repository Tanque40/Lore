#pragma once

#include "Lore/Application.h"
#include "Lore/Log.h"

extern Lore::Application* Lore::CreateApplication();

int main(int argc, const char** argv) {

	Lore::Log::Init();
	LR_CORE_WARN("Initialize log...");

	LR_CORE_INFO("Lore Engine Initialized!");
	auto app = Lore::CreateApplication();
	app->Run();
	delete app;

	return 0;
}
