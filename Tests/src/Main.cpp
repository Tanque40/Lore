#include "lrpch.h"

#include "TestCommon.h"

#include "Lore/Core/JobSystem.h"

namespace {
	int g_Failures = 0;
	int g_Checks = 0;
}

void Check(bool condition, const std::string& what) {
	g_Checks++;
	if (!condition) {
		g_Failures++;
		std::printf("  FALLA: %s\n", what.c_str());
	}
}

// Uso: MazeTests [--bench]
//   --bench  además mide el SVOBuilder nuevo contra el recursivo anterior en un mundo 256^3
//            (tarda; conviene correrlo en Release).
int main(int argc, char** argv) {
	Lore::Log::Init();
	Lore::Log::GetCoreLogger()->set_level(spdlog::level::warn);

	bool benchmark = false;
	for (int i = 1; i < argc; i++)
		if (std::string(argv[i]) == "--bench") benchmark = true;

	std::printf("== Laberintos ==\n");
	RunMazeTests();
	std::printf("== JobSystem ==\n");
	RunJobSystemTests();
	std::printf("== World (vóxeles y SVO) ==\n");
	RunWorldTests(benchmark);

	Lore::JobSystem::Shutdown();

	std::printf("\n%d de %d verificaciones pasaron\n", g_Checks - g_Failures, g_Checks);
	return g_Failures == 0 ? 0 : 1;
}
