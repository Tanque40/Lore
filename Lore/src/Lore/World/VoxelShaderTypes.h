// Tipos compartidos entre C++ y los shaders GLSL (Sandbox/src/Shaders/src/*.comp los
// incluyen con #include "Lore/World/VoxelShaderTypes.h"). Una sola definición evita que el
// layout de C++ y el de la GPU diverjan en silencio.
//
// Reglas para que el mismo texto valga en ambos lenguajes y el layout coincida en std140
// (OpenGL), en el MSL que genera spirv-cross (Metal) y en C++:
//   - sólo vec4, float y uint (nada de vec3: en std140 y en Metal se alinea a 16 bytes y
//     en C++ mide 12),
//   - los vec4 primero y los escalares después, en grupos de 4,
//   - el tamaño total múltiplo de 16 (los static_assert de abajo lo verifican).
#ifndef LORE_VOXEL_SHADER_TYPES_H
#define LORE_VOXEL_SHADER_TYPES_H

#ifdef __cplusplus
#include <cstdint>
#include <glm/glm.hpp>
namespace Lore::GPU {
	using vec4 = glm::vec4;
	using uint = uint32_t;
#endif

// Bindings fijos. En Metal, spirv-cross (--msl-decoration-binding) usa el mismo número como
// índice de [[buffer(n)]] / [[texture(n)]].
#define LORE_BINDING_SVO_NODES 0    // buffer de solo lectura con los SVONode
#define LORE_BINDING_FRAME_DATA 1   // uniform buffer FrameData
#define LORE_BINDING_OUTPUT_IMAGE 0 // imagen de salida del ray marcher

// Lo que antes eran uniforms sueltos (u_CameraPos, u_Fov, ...).
struct FrameData {
	vec4 CameraPos;   // xyz = posición (unidades de mundo), w sin usar
	vec4 CameraDir;   // xyz = dirección de vista
	vec4 CameraUp;
	vec4 CameraRight;
	float Fov;        // radianes
	float WorldSize;  // lado del SVO en vóxeles de índice
	float MaxLevels;  // log2(WorldSize)
	float VoxelScale; // unidades de mundo por vóxel
	float LodPixelThreshold;
	float LodCoverageThreshold;
	float _Pad0;
	float _Pad1;
};

// Mismo layout que SVO::SVONode (12 bytes, sin relleno en std430).
struct SVONodeGPU {
	uint descriptor;
	uint childIndex;
	uint material;
};

#ifdef __cplusplus
	static_assert(sizeof(FrameData) == 96, "FrameData debe medir 96 bytes (std140 / MSL)");
	static_assert(sizeof(FrameData) % 16 == 0, "FrameData debe ser múltiplo de 16 bytes");
	static_assert(sizeof(SVONodeGPU) == 12, "SVONodeGPU debe medir 12 bytes");
} // namespace Lore::GPU
#endif

#endif // LORE_VOXEL_SHADER_TYPES_H
