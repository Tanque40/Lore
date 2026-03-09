#version 460 core

// Definimos el tamaño del grupo de trabajo (hilos paralelos)
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// 1. Estructura idéntica al C++
struct SVONode {
	uint descriptor;
	uint material;
};

// 2. El Storage Buffer (SSBO) en el binding 0
layout(std430, binding = 0) readonly buffer SVOBuffer {
	SVONode nodes[];
};

// 3. La textura de salida en el binding 1
layout(rgba8, binding = 1) uniform writeonly image2D outputImage;

// Uniforms de la cámara
uniform vec3 u_CameraPos;
uniform vec3 u_CameraDir;
uniform vec3 u_CameraUp;
uniform vec3 u_CameraRight;
uniform float u_Fov;

// --- Función auxiliar para decodificar color ---
vec4 DecodificarColor(uint material) {
	float r = float(material & 0xFF) / 255.0;
	float g = float((material >> 8) & 0xFF) / 255.0;
	float b = float((material >> 16) & 0xFF) / 255.0;
	return vec4(r, g, b, 1.0);
}

void main() {
	float stepSize = 0.1; // Tamaño del paso para avanzar el rayo (ajustable)
	// Coordenada del píxel actual
	ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(outputImage);

	// Evitar procesar fuera de los límites de la pantalla
	if (pixelCoords.x >= dims.x || pixelCoords.y >= dims.y) return;

	// Coordenadas normalizadas (-1.0 a 1.0)
	vec2 uv = (vec2(pixelCoords) / vec2(dims)) * 2.0 - 1.0;

	// Aspect Ratio
	uv.x *= float(dims.x) / float(dims.y);

	// Generar la dirección del rayo (Ray Casting)
	vec3 rayDir = normalize(u_CameraDir + uv.x * u_CameraRight * u_Fov + uv.y * u_CameraUp * u_Fov);
	vec3 rayPos = u_CameraPos;

	vec4 colorFinal = vec4(0.1, 0.1, 0.1, 1.0); // Color de fondo (oscuridad de la cueva)

	// ====================================================
	// ALGORITMO DE RECORRIDO (TRAVERSAL BASE)
	// ====================================================
	// En un motor real, aquí usarías un "Stackless Traversal" o Ray Marching.
	// Para iniciar, este es el esqueleto iterativo de lectura:

	uint nodoActual = 0; // Empezamos en la raíz

	// Bucle conceptual de avance del rayo
	for (int pasos = 0; pasos < 100; ++pasos) {

		uint desc = nodes[nodoActual].descriptor;
		uint validMask = desc & 0xFF;
		uint leafMask = (desc >> 8) & 0xFF;
		uint childPtr = desc >> 16;

		// ¿Chocamos con una pared sólida?
		if (leafMask > 0) {
			colorFinal = DecodificarColor(nodes[nodoActual].material);

			// Efecto de sombreado simple por pasos (Falso AO)
			colorFinal.rgb *= (1.0 - float(pasos) * 0.01);
			break;
		}

		// ¿Llegamos a aire vacío? -> Avanzar el rayo matemáticamente
		// (La implementación matemática estricta de intersección AABB va aquí)
		 rayPos += rayDir * stepSize;

		// Lógica de descenso en el árbol...
		// uint octante = CalcularOctante(rayPos);
		// si existe (validMask & (1 << octante)), nodoActual = childPtr + offset;
	}

	// Escribir el píxel en la textura
	imageStore(outputImage, pixelCoords, colorFinal);
}
