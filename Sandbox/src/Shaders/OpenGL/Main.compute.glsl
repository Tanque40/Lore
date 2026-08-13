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

void main() {
	ivec2 gridPos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(outputTexture);

	// Límite de pantalla
	if (gridPos.x >= dims.x || gridPos.y >= dims.y) {
		return;
	}

	float gamma = 2.2;

	float worldSize = u_WorldSize;
	int maxLevels = int(u_MaxLevels);
	float stepSize = 0.1;
	int maxSteps = 250;

	// Calcular dirección del rayo
	vec2 uv = (vec2(gridPos) / vec2(dims)) * 2.0 - 1.0;
	uv.x *= float(dims.x) / float(dims.y);

	vec3 rayDir = normalize(u_CameraDir +
		uv.x * u_CameraRight * u_Fov +
		uv.y * u_CameraUp * u_Fov);

	vec3 rayPos = u_CameraPos;
	vec4 colorFinal = vec4(0.05, 0.05, 0.08, 1.0);
	bool hit = false;

	// BUCLE DE RAY MARCHING
	for (int paso = 0; paso < maxSteps; paso++) {

		if (any(lessThan(rayPos, vec3(0.0))) || any(greaterThan(rayPos, vec3(worldSize)))) {
			break;
		}

		uint nodoActual = 0u;
		vec3 posCajaMin = vec3(0.0);
		float tamanoActual = worldSize;

		for (int nivel = 0; nivel < maxLevels; nivel++) {
			uint desc = nodes[nodoActual].descriptor;
			uint validMask = desc & 0xFFu;
			uint leafMask = (desc >> 8) & 0xFFu;
			uint childPtr = desc >> 16;

			float halfSize = tamanoActual * 0.5;
			vec3 centro = posCajaMin + halfSize;

			uint bitX = rayPos.x >= centro.x ? 1u : 0u;
			uint bitY = rayPos.y >= centro.y ? 1u : 0u;
			uint bitZ = rayPos.z >= centro.z ? 1u : 0u;

			uint octante = bitX | (bitY << 1) | (bitZ << 2);
			uint mascaraOctante = 1u << octante;

			uint indiceHijo = childPtr + octante;
			colorFinal = DecodificarColor(nodes[indiceHijo].material);

			if ((leafMask & mascaraOctante) != 0u && colorFinal.a > 0.0) {
				// Esquinas de la caja del vóxel/nodo hoja que golpeamos, para derivar
				// la normal geométricamente (no se almacenan normales por nodo).
				vec3 boxMin = posCajaMin + vec3(bool(bitX) ? halfSize : 0.0, bool(bitY) ? halfSize : 0.0, bool(bitZ) ? halfSize : 0.0);
				vec3 boxMax = boxMin + halfSize;

				vec3 distToMin = rayPos - boxMin;
				vec3 distToMax = boxMax - rayPos;

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
				colorFinal.rgb *= lighting;

				// Atenuación por distancia (niebla) para poder juzgar profundidad
				float distanciaRecorrida = float(paso) * stepSize;
				float atenuacion = clamp(1.0 - distanciaRecorrida / (float(maxSteps) * stepSize), 0.0, 1.0);
				colorFinal.rgb *= atenuacion;

				colorFinal.rgb = pow(colorFinal.rgb, vec3(1.0 / gamma));
				hit = true;
				break;
			}

			if ((validMask & mascaraOctante) != 0u) {
				nodoActual = childPtr + octante;
				tamanoActual = halfSize;

				posCajaMin.x += bool(bitX) ? halfSize : 0.0;
				posCajaMin.y += bool(bitY) ? halfSize : 0.0;
				posCajaMin.z += bool(bitZ) ? halfSize : 0.0;
			} else {
				break; 
			}
		}

		if (hit) break;

		rayPos += rayDir * stepSize;
	}

	imageStore(outputTexture, gridPos, colorFinal);
}
