#include <metal_stdlib>
using namespace metal;

// 1. Estructuras (Debe coincidir con tu C++)
struct SVONode {
	// Bits 0-7: Valid, Bits 8-15: Leaf, Bits 16-23: Coverage (bits 24-31 sin usar).
	// Coverage es la fracción (0-255) de vóxeles sólidos en TODO el subárbol de este nodo;
	// material en un nodo rama guarda el color PROMEDIO de ese subárbol. Ambos sólo importan
	// para el LOD por distancia (ver TraceRay): renderizar el nodo como un bloque aproximado
	// en vez de bajar hasta el detalle exacto cuando ya está lejos.
	uint descriptor;
	uint childIndex; // Rango completo de 32 bits (no empaquetado en descriptor)
	uint material;
};

// Uniforms de cámara y configuración
struct Uniforms {
	float3 cameraPos;
	float3 cameraDir;
	float3 cameraUp;
	float3 cameraRight;
	float fov;
	float worldSize;
	float maxLevels;
	float voxelScale;
	float lodPixelThreshold;
	float lodCoverageThreshold;
};

// Función auxiliar para decodificar color
inline float4 DecodificarColor(uint material) {
	float r = float(material & 0xFF) / 255.0;
	float g = float((material >> 8) & 0xFF) / 255.0;
	float b = float((material >> 16) & 0xFF) / 255.0;
	float a = float((material >> 24) & 0xFF) / 255.0; // Si usas el byte alfa para algo

	if (a == 0.0)
		return float4(r, g, b, a);

	return float4(r, g, b, 1.0f);
}

// Distancia a la que el rayo sale de una caja AABB, asumiendo que "origin" ya está
// dentro de ella. Se usa para saltar de un salto todo un nodo vacío del octree en
// vez de avanzar a pasos fijos pequeños (ray marching adaptativo).
inline float AABBExitT(float3 origin, float3 dir, float3 boxMin, float3 boxMax) {
	float tx = dir.x > 0.0 ? (boxMax.x - origin.x) / dir.x : (dir.x < 0.0 ? (boxMin.x - origin.x) / dir.x : 1e30f);
	float ty = dir.y > 0.0 ? (boxMax.y - origin.y) / dir.y : (dir.y < 0.0 ? (boxMin.y - origin.y) / dir.y : 1e30f);
	float tz = dir.z > 0.0 ? (boxMax.z - origin.z) / dir.z : (dir.z < 0.0 ? (boxMin.z - origin.z) / dir.z : 1e30f);
	return min(tx, min(ty, tz));
}

// Normal aproximada de la caja de un nodo/vóxel a partir de a qué cara está más cerca el
// punto de impacto (no hay normales guardadas por nodo). Se reusa tanto para un impacto
// exacto contra una hoja como para un impacto "impostor" de LOD contra un nodo rama.
inline float3 BoxNormal(float3 indexPos, float3 boxMin, float3 boxMax) {
	float3 distToMin = indexPos - boxMin;
	float3 distToMax = boxMax - indexPos;

	float3 normal = float3(-1.0, 0.0, 0.0);
	float minDist = distToMin.x;
	if (distToMin.y < minDist) { minDist = distToMin.y; normal = float3(0.0, -1.0, 0.0); }
	if (distToMin.z < minDist) { minDist = distToMin.z; normal = float3(0.0, 0.0, -1.0); }
	if (distToMax.x < minDist) { minDist = distToMax.x; normal = float3(1.0, 0.0, 0.0); }
	if (distToMax.y < minDist) { minDist = distToMax.y; normal = float3(0.0, 1.0, 0.0); }
	if (distToMax.z < minDist) { minDist = distToMax.z; normal = float3(0.0, 0.0, 1.0); }
	return normal;
}

// Luz "linterna" + niebla + gamma para un color de impacto ya resuelto. Compartido entre
// el impacto exacto contra una hoja y el impacto aproximado de LOD contra un nodo rama.
inline float4 ShadeHit(float4 baseColor, float3 normal, float3 rayDir, float distanciaRecorrida, float maxRenderDistance, float gamma) {
	float ambient = 0.12;
	float diffuse = max(dot(normal, -rayDir), 0.0);
	float lighting = ambient + (1.0 - ambient) * diffuse;
	baseColor.rgb *= lighting;

	float atenuacion = clamp(1.0 - distanciaRecorrida / maxRenderDistance, 0.0, 1.0);
	baseColor.rgb *= atenuacion;

	baseColor.rgb = pow(baseColor.rgb, float3(1.0 / gamma));
	return baseColor;
}

// Traza un único rayo contra el SVO y devuelve el color final (con luz e iluminación
// ya aplicadas). Separado del kernel para poder llamarlo varias veces por píxel
// (supersampling) sin duplicar la lógica.
inline float4 TraceRay(float3 rayOrigin, float3 rayDir, device const SVONode* nodes, float worldSizeIndex, int maxLevels, float voxelScale, float lodPixelThreshold, float lodCoverageThreshold) {
	float gamma = 2.2;
	float worldSizeWorld = worldSizeIndex * voxelScale; // Límite del mundo en unidades de mundo
	float maxRenderDistance = 40.0f; // Distancia (en unidades de mundo) a la que todo se pierde en niebla
	float epsilon = max(voxelScale * 0.01f, 0.0005f); // Empujoncito para no quedar pegado al borde de una caja

	float3 rayPos = rayOrigin;
	float4 colorFinal = float4(0.05, 0.05, 0.08, 1.0); // Color de fondo (oscuridad)
	bool hit = false;

	const int maxSteps = 128; // Salvaguarda: ahora cada paso puede saltar un nodo entero, no un texel fijo
	for (int paso = 0; paso < maxSteps; paso++) {

		// 1. Verificar si el rayo se salió del mundo (Culling de límites)
		if (rayPos.x < 0.0 || rayPos.y < 0.0 || rayPos.z < 0.0 ||
			rayPos.x > worldSizeWorld || rayPos.y > worldSizeWorld || rayPos.z > worldSizeWorld) {
			break; // Salió de la cueva al vacío infinito
		}

		float distanciaRecorrida = length(rayPos - rayOrigin);
		if (distanciaRecorrida > maxRenderDistance) {
			break;
		}

		// El octree vive en "espacio de índice" (vóxeles), el rayo avanza en unidades
		// de mundo: convertimos una vez por paso.
		float3 indexPos = rayPos / voxelScale;

		// 2. Bajar por el árbol (Top-Down Traversal)
		uint nodoActual = 0; // Empezar siempre en la raíz
		float3 posCajaMin = float3(0.0);
		float tamañoActual = worldSizeIndex;
		bool avanzo = false;

		for (int nivel = 0; nivel < maxLevels; nivel++) {
			uint desc = nodes[nodoActual].descriptor;
			uint validMask = desc & 0xFF;
			uint leafMask = (desc >> 8) & 0xFF;
			uint coverage = (desc >> 16) & 0xFF;
			uint childPtr = nodes[nodoActual].childIndex;

			// LOD por distancia: si la caja de este nodo ya se ve más chica en pantalla que
			// lodPixelThreshold (ángulo aproximado, tamaño/distancia) no vale la pena bajar
			// más — se resolvería un detalle que a esa distancia no se distinguiría. En vez
			// de eso tratamos TODO el nodo como un único bloque usando el color promedio de
			// su subárbol (si tiene suficiente cobertura sólida) o como aire (si no).
			// nivel > 0 evita colapsar la raíz entera en un solo bloque.
			float nodeWorldSize = tamañoActual * voxelScale;
			if (nivel > 0 && lodPixelThreshold > 0.0 && nodeWorldSize < distanciaRecorrida * lodPixelThreshold) {
				float3 boxMax = posCajaMin + tamañoActual;
				if (float(coverage) > lodCoverageThreshold * 255.0) {
					float3 normal = BoxNormal(indexPos, posCajaMin, boxMax);
					colorFinal = ShadeHit(DecodificarColor(nodes[nodoActual].material), normal, rayDir, distanciaRecorrida, maxRenderDistance, gamma);
					hit = true;
				} else {
					float3 emptyMinWorld = posCajaMin * voxelScale;
					float3 emptyMaxWorld = boxMax * voxelScale;
					float tExit = AABBExitT(rayPos, rayDir, emptyMinWorld, emptyMaxWorld);
					rayPos += rayDir * max(tExit + epsilon, epsilon);
					avanzo = true;
				}
				break;
			}

			// A. ¿En qué octante de esta caja está la punta del rayo?
			float halfSize = tamañoActual * 0.5;
			float3 centro = posCajaMin + halfSize;

			uint bitX = indexPos.x >= centro.x ? 1 : 0;
			uint bitY = indexPos.y >= centro.y ? 1 : 0;
			uint bitZ = indexPos.z >= centro.z ? 1 : 0;

			// Usamos la misma codificación Morton (Z-Curve) que en C++
			uint octante = bitX | (bitY << 1) | (bitZ << 2);
			uint mascaraOctante = 1 << octante;

			uint indiceHijo = childPtr + octante;

			// Caja del octante actual, en espacio de índice
			float3 childBoxMin = posCajaMin + float3(bitX ? halfSize : 0.0, bitY ? halfSize : 0.0, bitZ ? halfSize : 0.0);
			float3 childBoxMax = childBoxMin + halfSize;

			// B. ¿Es sólido el octante actual?
			if ((leafMask & mascaraOctante) != 0) {
				float4 hitColor = DecodificarColor(nodes[indiceHijo].material);
				if (hitColor.a > 0.0) {
					// ¡Chocamos con la pared de la cueva!
					float3 normal = BoxNormal(indexPos, childBoxMin, childBoxMax);
					colorFinal = ShadeHit(hitColor, normal, rayDir, distanciaRecorrida, maxRenderDistance, gamma);
					hit = true;
					break; // Romper el bucle de niveles
				}
			}

			// C. ¿Tiene más subdivisiones? (Es una rama mezclada)
			if ((validMask & mascaraOctante) != 0) {
				nodoActual = indiceHijo;
				tamañoActual = halfSize;
				posCajaMin = childBoxMin;
			}
			else {
				// D. Es aire puro: en vez de avanzar un paso fijo, saltamos de una vez
				// hasta el borde de salida de esta caja vacía (ray marching adaptativo).
				float3 emptyBoxMinWorld = childBoxMin * voxelScale;
				float3 emptyBoxMaxWorld = childBoxMax * voxelScale;
				float tExit = AABBExitT(rayPos, rayDir, emptyBoxMinWorld, emptyBoxMaxWorld);
				rayPos += rayDir * max(tExit + epsilon, epsilon);
				avanzo = true;
				break; // Romper el bucle de niveles para reiniciar el descenso desde la raíz
			}
		}

		if (hit) {
			break;
		}

		if (!avanzo) {
			// Salvaguarda defensiva (no debería ocurrir con un octree bien formado):
			// evita quedarnos congelados si ni impactamos ni saltamos.
			rayPos += rayDir * epsilon;
		}
	}

	return colorFinal;
}

// 2. El Kernel Principal
kernel void compute_main(
	texture2d<float, access::write> outputTexture [[texture(0)]],
	device const SVONode* nodes [[buffer(0)]],
	constant Uniforms& uniforms [[buffer(1)]],
	uint2 gridPos [[thread_position_in_grid]] )
{
	// Límite de pantalla
	if (gridPos.x >= outputTexture.get_width() || gridPos.y >= outputTexture.get_height()) {
		return;
	}

	float worldSize = uniforms.worldSize; // Tamaño real del VoxelGrid/SVO, en vóxeles de índice
	int maxLevels = int(uniforms.maxLevels); // log2(worldSize): profundidad real del octree
	float voxelScale = uniforms.voxelScale;
	float lodPixelThreshold = uniforms.lodPixelThreshold;
	float lodCoverageThreshold = uniforms.lodCoverageThreshold;

	float2 dims = float2(outputTexture.get_width(), outputTexture.get_height());

	// Supersampling 2x2: disparamos 4 rayos con sub-desplazamiento dentro del píxel y
	// promediamos el color, para suavizar los bordes en diente de sierra de las
	// aristas de los vóxeles sin necesitar un paso de post-procesado aparte.
	const int kSamples = 2;
	float4 colorAccum = float4(0.0);

	for (int sy = 0; sy < kSamples; sy++) {
		for (int sx = 0; sx < kSamples; sx++) {
			float2 subOffset = (float2(sx, sy) + 0.5) / float(kSamples) - 0.5;
			float2 uv = ((float2(gridPos) + subOffset) / dims) * 2.0 - 1.0;
			uv.x *= dims.x / dims.y;

			float3 rayDir = normalize(uniforms.cameraDir +
				uv.x * uniforms.cameraRight * uniforms.fov +
				uv.y * uniforms.cameraUp * uniforms.fov);

			colorAccum += TraceRay(uniforms.cameraPos, rayDir, nodes, worldSize, maxLevels, voxelScale, lodPixelThreshold, lodCoverageThreshold);
		}
	}

	float4 colorFinal = colorAccum / float(kSamples * kSamples);

	// Escribir en la textura de destino
	outputTexture.write(colorFinal, gridPos);
}
