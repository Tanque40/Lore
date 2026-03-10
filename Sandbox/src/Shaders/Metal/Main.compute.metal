#include <metal_stdlib>
using namespace metal;

// 1. Estructura idéntica al C++
struct SVONode {
    uint descriptor;
    uint material;
};

// 2. Parámetros globales (Cámara)
struct Uniforms {
    float3 cameraPos;
    float3 cameraDir;
    float3 cameraUp;
    float3 cameraRight;
    float fov;
};

// --- Función auxiliar ---
float4 DecodificarColor(uint material) {
    float r = float(material & 0xFF) / 255.0;
    float g = float((material >> 8) & 0xFF) / 255.0;
    float b = float((material >> 16) & 0xFF) / 255.0;
    return float4(r, g, b, 1.0);
}

// 3. El Kernel Function (Compute Shader)
kernel void compute_main(
    texture2d<float, access::write> outputTexture [[texture(0)]],
    device const SVONode* nodes [[buffer(0)]],
    constant Uniforms& uniforms [[buffer(1)]],
    uint2 gridPos [[thread_position_in_grid]])
{
    // Limitar al tamaño de la pantalla
    if (gridPos.x >= outputTexture.get_width() || gridPos.y >= outputTexture.get_height()) {
        return;
    }

    float2 dims = float2(outputTexture.get_width(), outputTexture.get_height());
    float2 uv = (float2(gridPos) / dims) * 2.0 - 1.0;
    uv.x *= dims.x / dims.y;

    float3 rayDir = normalize(uniforms.cameraDir + uv.x * uniforms.cameraRight * uniforms.fov + uv.y * uniforms.cameraUp * uniforms.fov);
    float3 rayPos = uniforms.cameraPos;

    float4 colorFinal = float4(0.1, 0.1, 0.1, 1.0);
    uint nodoActual = 0;

    for (int pasos = 0; pasos < 100; ++pasos) {

        uint desc = nodes[nodoActual].descriptor;
        uint validMask = desc & 0xFF;
        uint leafMask = (desc >> 8) & 0xFF;
        uint childPtr = desc >> 16;

        if (leafMask > 0) {
            colorFinal = DecodificarColor(nodes[nodoActual].material);
            colorFinal.rgb *= (1.0 - float(pasos) * 0.01);
            break;
        }

        // Aquí iría la misma matemática de avance e intersección de cajas
    }

    // Escribir en la textura
    outputTexture.write(colorFinal, gridPos);
}
