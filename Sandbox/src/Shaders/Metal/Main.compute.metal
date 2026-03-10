#include <metal_stdlib>
using namespace metal;

// 1. Estructuras (Debe coincidir con tu C++)
struct SVONode {
    uint descriptor;
    uint material;
};

// Uniforms de cámara y configuración
struct Uniforms {
    float3 cameraPos;
    float3 cameraDir;
    float3 cameraUp;
    float3 cameraRight;
    float fov;
};

// Función auxiliar para decodificar color
inline float4 DecodificarColor(uint material) {
    float r = float(material & 0xFF) / 255.0;
    float g = float((material >> 8) & 0xFF) / 255.0;
    float b = float((material >> 16) & 0xFF) / 255.0;
    return float4(r, g, b, 1.0);
}

// 2. El Kernel Principal
kernel void compute_main(
    texture2d<float, access::write> outputTexture [[texture(0)]],
    device const SVONode* nodes [[buffer(0)]],
    constant Uniforms& uniforms [[buffer(1)]],
    uint2 gridPos [[thread_position_in_grid]])
{
	// Configuración del Ray Marching
    float worldSize = 64.0f; // Tamaño del VoxelGrid original (ej. 64.0)
    float stepSize = 0.5f;  // Qué tan pequeño es el paso (ej. 0.25)
    int maxSteps = 150;    // Máximo de pasos antes de rendirse (ej. 400)

    // Límite de pantalla
    if (gridPos.x >= outputTexture.get_width() || gridPos.y >= outputTexture.get_height()) {
        return;
    }

    // Calcular dirección del rayo para este píxel
    float2 dims = float2(outputTexture.get_width(), outputTexture.get_height());
    float2 uv = (float2(gridPos) / dims) * 2.0 - 1.0;
    uv.x *= dims.x / dims.y;

    float3 rayDir = normalize(uniforms.cameraDir +
                              uv.x * uniforms.cameraRight * uniforms.fov +
                              uv.y * uniforms.cameraUp * uniforms.fov);

    // Posición inicial del rayo (avanzaremos esta variable)
    float3 rayPos = uniforms.cameraPos;

    float4 colorFinal = float4(0.05, 0.05, 0.08, 1.0); // Color de fondo (oscuridad)
    bool hit = false;

    // ====================================================
    // BUCLE DE AVANCE (RAY MARCHING)
    // ====================================================
    for (int paso = 0; paso < maxSteps; paso++) {

        // 1. Verificar si el rayo se salió del mundo (Culling de límites)
        if (rayPos.x < 0.0 || rayPos.y < 0.0 || rayPos.z < 0.0 ||
            rayPos.x > worldSize || rayPos.y > worldSize || rayPos.z > worldSize) {
            break; // Salió de la cueva al vacío infinito
        }

        // 2. Bajar por el árbol (Top-Down Traversal)
        uint nodoActual = 0; // Empezar siempre en la raíz
        float3 posCajaMin = float3(0.0);
        float tamañoActual = worldSize;

        // Asumimos un máximo de 10 subdivisiones (suficiente para un grid de 1024)
        for (int nivel = 0; nivel < 10; nivel++) {
            uint desc = nodes[nodoActual].descriptor;
            uint validMask = desc & 0xFF;
            uint leafMask = (desc >> 8) & 0xFF;
            uint childPtr = desc >> 16;

            // A. ¿En qué octante de esta caja está la punta del rayo?
            float halfSize = tamañoActual * 0.5;
            float3 centro = posCajaMin + halfSize;

            uint bitX = rayPos.x >= centro.x ? 1 : 0;
            uint bitY = rayPos.y >= centro.y ? 1 : 0;
            uint bitZ = rayPos.z >= centro.z ? 1 : 0;

            // Usamos la misma codificación Morton (Z-Curve) que en C++
            uint octante = bitX | (bitY << 1) | (bitZ << 2);
            uint mascaraOctante = 1 << octante;

            // B. ¿Es sólido el octante actual?
            if ((leafMask & mascaraOctante) != 0) {
                // ¡Chocamos con la pared de la cueva!
                uint indiceHijo = childPtr + octante;
                colorFinal = DecodificarColor(nodes[indiceHijo].material);

                // Oclusión ambiental falsa basada en distancia/pasos
                colorFinal.rgb *= (1.0 - (float(paso) / float(maxSteps)));
                hit = true;
                break; // Romper el bucle de niveles
            }

            // C. ¿Tiene más subdivisiones? (Es una rama mezclada)
            if ((validMask & mascaraOctante) != 0) {
                nodoActual = childPtr + octante;
                tamañoActual = halfSize;

                // Actualizar la esquina inferior izquierda de la caja para el siguiente nivel
                posCajaMin.x += bitX ? halfSize : 0.0;
                posCajaMin.y += bitY ? halfSize : 0.0;
                posCajaMin.z += bitZ ? halfSize : 0.0;
            } else {
                // D. Es aire puro.
                break; // Romper el bucle de niveles para dejar que el rayo avance
            }
        }

        // 3. Si chocamos, terminamos el rayo principal
        if (hit) {
            break;
        }

        // 4. Si era aire, avanzamos el rayo para la siguiente iteración
        rayPos += rayDir * stepSize;
    }

    // Escribir en la textura de destino
    outputTexture.write(colorFinal, gridPos);
}
