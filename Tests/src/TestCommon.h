#pragma once

// Pruebas del motor sin ventana ni GPU (ProcGen, World y JobSystem). No usan ningún
// framework: el ejecutable devuelve 0 si todo pasa y 1 si algo falla, para poder correrlo
// en CI o a mano (ver Tests/premake5.lua y .github/workflows/tests.yml).

#include <cstdio>
#include <string>

void Check(bool condition, const std::string& what);

void RunMazeTests();
void RunWorldTests(bool benchmark);
void RunJobSystemTests();
