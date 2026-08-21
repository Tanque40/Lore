#version 460 core

// 1. Configuración del grupo de hilos (Workgroup size)
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Estructuras
struct SVONode {
	uint descriptor;
	uint material;
};

// Buffers y Texturas
layout(rgba8, binding = 0) uniform writeonly image2D outputTexture;

layout(std430, binding = 0) readonly buffer NodeBuffer {
	SVONode nodes[];
};

uniform vec3 u_CameraPos;
uniform vec3 u_CameraDir;
uniform vec3 u_CameraUp;
uniform vec3 u_CameraRight;
uniform float u_Fov;
uniform float u_WorldSize;
uniform float u_MaxLevels;
uniform float u_VoxelScale;

// Función auxiliar para decodificar color
vec4 DecodificarColor(uint material) {
	float r = float(material & 0xFF) / 255.0;
	float g = float((material >> 8) & 0xFF) / 255.0;
	float b = float((material >> 16) & 0xFF) / 255.0;
	float a = float((material >> 24) & 0xFF) / 255.0;

	if (a == 0.0)
		return vec4(r, g, b, a);

	return vec4(r, g, b, 1.0);
}

// Distancia a la que el rayo sale de una caja AABB, asumiendo que "origin" ya está
// dentro de ella. Se usa para saltar de un salto todo un nodo vacío del octree en
// vez de avanzar a pasos fijos pequeños (ray marching adaptativo).
float AABBExitT(vec3 origin, vec3 dir, vec3 boxMin, vec3 boxMax) {
	float tx = dir.x > 0.0 ? (boxMax.x - origin.x) / dir.x : (dir.x < 0.0 ? (boxMin.x - origin.x) / dir.x : 1e30);
	float ty = dir.y > 0.0 ? (boxMax.y - origin.y) / dir.y : (dir.y < 0.0 ? (boxMin.y - origin.y) / dir.y : 1e30);
	float tz = dir.z > 0.0 ? (boxMax.z - origin.z) / dir.z : (dir.z < 0.0 ? (boxMin.z - origin.z) / dir.z : 1e30);
	return min(tx, min(ty, tz));
}

// Traza un único rayo contra el SVO y devuelve el color final (con luz y niebla ya
// aplicadas). Separado de main() para poder llamarlo varias veces por píxel
// (supersampling) sin duplicar la lógica.
vec4 TraceRay(vec3 rayOrigin, vec3 rayDir, float worldSizeIndex, int maxLevels, float voxelScale) {
	float gamma = 2.2;
	float worldSizeWorld = worldSizeIndex * voxelScale; // Límite del mundo en unidades de mundo
	float maxRenderDistance = 40.0; // Distancia (en unidades de mundo) a la que todo se pierde en niebla
	float epsilon = max(voxelScale * 0.01, 0.0005);

	vec3 rayPos = rayOrigin;
	vec4 colorFinal = vec4(0.05, 0.05, 0.08, 1.0);
	bool hit = false;

	const int maxSteps = 128; // Salvaguarda: ahora cada paso puede saltar un nodo entero, no un texel fijo
	for (int paso = 0; paso < maxSteps; paso++) {

		if (any(lessThan(rayPos, vec3(0.0))) || any(greaterThan(rayPos, vec3(worldSizeWorld)))) {
			break;
		}

		float distanciaRecorrida = length(rayPos - rayOrigin);
		if (distanciaRecorrida > maxRenderDistance) {
			break;
		}

		// El octree vive en "espacio de índice" (vóxeles), el rayo avanza en unidades
		// de mundo: convertimos una vez por paso.
		vec3 indexPos = rayPos / voxelScale;

		uint nodoActual = 0u;
		vec3 posCajaMin = vec3(0.0);
		float tamanoActual = worldSizeIndex;
		bool avanzo = false;

		for (int nivel = 0; nivel < maxLevels; nivel++) {
			uint desc = nodes[nodoActual].descriptor;
			uint validMask = desc & 0xFFu;
			uint leafMask = (desc >> 8) & 0xFFu;
			uint childPtr = desc >> 16;

			float halfSize = tamanoActual * 0.5;
			vec3 centro = posCajaMin + halfSize;

			uint bitX = indexPos.x >= centro.x ? 1u : 0u;
			uint bitY = indexPos.y >= centro.y ? 1u : 0u;
			uint bitZ = indexPos.z >= centro.z ? 1u : 0u;

			uint octante = bitX | (bitY << 1) | (bitZ << 2);
			uint mascaraOctante = 1u << octante;

			uint indiceHijo = childPtr + octante;

			vec3 childBoxMin = posCajaMin + vec3(bool(bitX) ? halfSize : 0.0, bool(bitY) ? halfSize : 0.0, bool(bitZ) ? halfSize : 0.0);
			vec3 childBoxMax = childBoxMin + halfSize;

			if ((leafMask & mascaraOctante) != 0u) {
				vec4 hitColor = DecodificarColor(nodes[indiceHijo].material);
				if (hitColor.a > 0.0) {
					// Esquinas de la caja del vóxel/nodo hoja que golpeamos, para derivar
					// la normal geométricamente (no se almacenan normales por nodo).
					vec3 distToMin = indexPos - childBoxMin;
					vec3 distToMax = childBoxMax - indexPos;

					vec3 normal = vec3(-1.0, 0.0, 0.0);
					float minDist = distToMin.x;
					if (distToMin.y < minDist) { minDist = distToMin.y; normal = vec3(0.0, -1.0, 0.0); }
					if (distToMin.z < minDist) { minDist = distToMin.z; normal = vec3(0.0, 0.0, -1.0); }
					if (distToMax.x < minDist) { minDist = distToMax.x; normal = vec3(1.0, 0.0, 0.0); }
					if (distToMax.y < minDist) { minDist = distToMax.y; normal = vec3(0.0, 1.0, 0.0); }
					if (distToMax.z < minDist) { minDist = distToMax.z; normal = vec3(0.0, 0.0, 1.0); }

					// Luz "linterna" pegada a la cámara: como el rayo parte de la cámara,
					// la dirección hacia la luz desde el punto de impacto es siempre -rayDir.
					float ambient = 0.12;
					float diffuse = max(dot(normal, -rayDir), 0.0);
					float lighting = ambient + (1.0 - ambient) * diffuse;
					hitColor.rgb *= lighting;

					// Atenuación por distancia (niebla) para poder juzgar profundidad
					float atenuacion = clamp(1.0 - distanciaRecorrida / maxRenderDistance, 0.0, 1.0);
					hitColor.rgb *= atenuacion;

					hitColor.rgb = pow(hitColor.rgb, vec3(1.0 / gamma));

					colorFinal = hitColor;
					hit = true;
					break;
				}
			}

			if ((validMask & mascaraOctante) != 0u) {
				nodoActual = indiceHijo;
				tamanoActual = halfSize;
				posCajaMin = childBoxMin;
			} else {
				// Es aire puro: en vez de avanzar un paso fijo, saltamos de una vez hasta
				// el borde de salida de esta caja vacía (ray marching adaptativo).
				vec3 emptyBoxMinWorld = childBoxMin * voxelScale;
				vec3 emptyBoxMaxWorld = childBoxMax * voxelScale;
				float tExit = AABBExitT(rayPos, rayDir, emptyBoxMinWorld, emptyBoxMaxWorld);
				rayPos += rayDir * max(tExit + epsilon, epsilon);
				avanzo = true;
				break;
			}
		}

		if (hit) break;

		if (!avanzo) {
			// Salvaguarda defensiva (no debería ocurrir con un octree bien formado).
			rayPos += rayDir * epsilon;
		}
	}

	return colorFinal;
}

void main() {
	ivec2 gridPos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(outputTexture);

	// Límite de pantalla
	if (gridPos.x >= dims.x || gridPos.y >= dims.y) {
		return;
	}

	float worldSize = u_WorldSize;
	int maxLevels = int(u_MaxLevels);
	float voxelScale = u_VoxelScale;

	// Supersampling 2x2: disparamos 4 rayos con sub-desplazamiento dentro del píxel y
	// promediamos el color, para suavizar los bordes en diente de sierra de las
	// aristas de los vóxeles sin necesitar un paso de post-procesado aparte.
	const int kSamples = 2;
	vec4 colorAccum = vec4(0.0);

	for (int sy = 0; sy < kSamples; sy++) {
		for (int sx = 0; sx < kSamples; sx++) {
			vec2 subOffset = (vec2(sx, sy) + 0.5) / float(kSamples) - 0.5;
			vec2 uv = ((vec2(gridPos) + subOffset) / vec2(dims)) * 2.0 - 1.0;
			uv.x *= float(dims.x) / float(dims.y);

			vec3 rayDir = normalize(u_CameraDir +
				uv.x * u_CameraRight * u_Fov +
				uv.y * u_CameraUp * u_Fov);

			colorAccum += TraceRay(u_CameraPos, rayDir, worldSize, maxLevels, voxelScale);
		}
	}

	vec4 colorFinal = colorAccum / float(kSamples * kSamples);

	imageStore(outputTexture, gridPos, colorFinal);
}
