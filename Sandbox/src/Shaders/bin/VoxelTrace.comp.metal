#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SVONodeGPU
{
    uint descriptor;
    uint childIndex;
    uint material;
};

struct NodeBuffer
{
    SVONodeGPU nodes[1];
};

struct FrameData
{
    float4 CameraPos;
    float4 CameraDir;
    float4 CameraUp;
    float4 CameraRight;
    float Fov;
    float WorldSize;
    float MaxLevels;
    float VoxelScale;
    float LodPixelThreshold;
    float LodCoverageThreshold;
    float _Pad0;
    float _Pad1;
};

struct FrameBlock
{
    FrameData u_Frame;
};

constant uint3 gl_WorkGroupSize [[maybe_unused]] = uint3(16u, 16u, 1u);

static inline __attribute__((always_inline))
float3 BoxNormal(thread const float3& indexPos, thread const float3& boxMin, thread const float3& boxMax)
{
    float3 distToMin = indexPos - boxMin;
    float3 distToMax = boxMax - indexPos;
    float3 normal = float3(-1.0, 0.0, 0.0);
    float minDist = distToMin.x;
    if (distToMin.y < minDist)
    {
        minDist = distToMin.y;
        normal = float3(0.0, -1.0, 0.0);
    }
    if (distToMin.z < minDist)
    {
        minDist = distToMin.z;
        normal = float3(0.0, 0.0, -1.0);
    }
    if (distToMax.x < minDist)
    {
        minDist = distToMax.x;
        normal = float3(1.0, 0.0, 0.0);
    }
    if (distToMax.y < minDist)
    {
        minDist = distToMax.y;
        normal = float3(0.0, 1.0, 0.0);
    }
    if (distToMax.z < minDist)
    {
        minDist = distToMax.z;
        normal = float3(0.0, 0.0, 1.0);
    }
    return normal;
}

static inline __attribute__((always_inline))
float4 DecodificarColor(thread const uint& material)
{
    float r = float(material & 255u) / 255.0;
    float g = float((material >> uint(8)) & 255u) / 255.0;
    float b = float((material >> uint(16)) & 255u) / 255.0;
    float a = float((material >> uint(24)) & 255u) / 255.0;
    if (a == 0.0)
    {
        return float4(r, g, b, a);
    }
    return float4(r, g, b, 1.0);
}

static inline __attribute__((always_inline))
float4 ShadeHit(thread float4& baseColor, thread const float3& normal, thread const float3& rayDir, thread const float& distanciaRecorrida, thread const float& maxRenderDistance, thread const float& gamma)
{
    float ambient = 0.119999997317790985107421875;
    float diffuse = fast::max(dot(normal, -rayDir), 0.0);
    float lighting = ambient + ((1.0 - ambient) * diffuse);
    float4 _287 = baseColor;
    float3 _289 = _287.xyz * lighting;
    baseColor.x = _289.x;
    baseColor.y = _289.y;
    baseColor.z = _289.z;
    float atenuacion = fast::clamp(1.0 - (distanciaRecorrida / maxRenderDistance), 0.0, 1.0);
    float4 _303 = baseColor;
    float3 _305 = _303.xyz * atenuacion;
    baseColor.x = _305.x;
    baseColor.y = _305.y;
    baseColor.z = _305.z;
    float4 _312 = baseColor;
    float3 _317 = pow(_312.xyz, float3(1.0 / gamma));
    baseColor.x = _317.x;
    baseColor.y = _317.y;
    baseColor.z = _317.z;
    return baseColor;
}

static inline __attribute__((always_inline))
float AABBExitT(thread const float3& origin, thread const float3& dir, thread const float3& boxMin, thread const float3& boxMax)
{
    float _104;
    if (dir.x > 0.0)
    {
        _104 = (boxMax.x - origin.x) / dir.x;
    }
    else
    {
        float _119;
        if (dir.x < 0.0)
        {
            _119 = (boxMin.x - origin.x) / dir.x;
        }
        else
        {
            _119 = 1000000015047466219876688855040.0;
        }
        _104 = _119;
    }
    float tx = _104;
    float _139;
    if (dir.y > 0.0)
    {
        _139 = (boxMax.y - origin.y) / dir.y;
    }
    else
    {
        float _154;
        if (dir.y < 0.0)
        {
            _154 = (boxMin.y - origin.y) / dir.y;
        }
        else
        {
            _154 = 1000000015047466219876688855040.0;
        }
        _139 = _154;
    }
    float ty = _139;
    float _173;
    if (dir.z > 0.0)
    {
        _173 = (boxMax.z - origin.z) / dir.z;
    }
    else
    {
        float _188;
        if (dir.z < 0.0)
        {
            _188 = (boxMin.z - origin.z) / dir.z;
        }
        else
        {
            _188 = 1000000015047466219876688855040.0;
        }
        _173 = _188;
    }
    float tz = _173;
    return fast::min(tx, fast::min(ty, tz));
}

static inline __attribute__((always_inline))
float4 TraceRay(thread const float3& rayOrigin, thread const float3& rayDir, thread const float& worldSizeIndex, thread const int& maxLevels, thread const float& voxelScale, thread const float& lodPixelThreshold, thread const float& lodCoverageThreshold, const device NodeBuffer& v_412)
{
    float gamma = 2.2000000476837158203125;
    float worldSizeWorld = worldSizeIndex * voxelScale;
    float maxRenderDistance = 40.0;
    float epsilon = fast::max(voxelScale * 0.00999999977648258209228515625, 0.0005000000237487256526947021484375);
    float3 rayPos = rayOrigin;
    float4 colorFinal = float4(0.0500000007450580596923828125, 0.0500000007450580596923828125, 0.07999999821186065673828125, 1.0);
    bool hit = false;
    for (int paso = 0; paso < 128; paso++)
    {
        bool _364 = any(rayPos < float3(0.0));
        bool _373;
        if (!_364)
        {
            _373 = any(rayPos > float3(worldSizeWorld));
        }
        else
        {
            _373 = _364;
        }
        if (_373)
        {
            break;
        }
        float distanciaRecorrida = length(rayPos - rayOrigin);
        if (distanciaRecorrida > maxRenderDistance)
        {
            break;
        }
        float3 indexPos = rayPos / float3(voxelScale);
        uint nodoActual = 0u;
        float3 posCajaMin = float3(0.0);
        float tamanoActual = worldSizeIndex;
        bool avanzo = false;
        for (int nivel = 0; nivel < maxLevels; nivel++)
        {
            uint desc = v_412.nodes[nodoActual].descriptor;
            uint validMask = desc & 255u;
            uint leafMask = (desc >> uint(8)) & 255u;
            uint coverage = (desc >> uint(16)) & 255u;
            uint childPtr = v_412.nodes[nodoActual].childIndex;
            float nodeWorldSize = tamanoActual * voxelScale;
            bool _441 = (nivel > 0) && (lodPixelThreshold > 0.0);
            bool _449;
            if (_441)
            {
                _449 = nodeWorldSize < (distanciaRecorrida * lodPixelThreshold);
            }
            else
            {
                _449 = _441;
            }
            if (_449)
            {
                float3 boxMax = posCajaMin + float3(tamanoActual);
                if (float(coverage) > (lodCoverageThreshold * 255.0))
                {
                    float3 param = indexPos;
                    float3 param_1 = posCajaMin;
                    float3 param_2 = boxMax;
                    float3 normal = BoxNormal(param, param_1, param_2);
                    uint param_3 = v_412.nodes[nodoActual].material;
                    float4 param_4 = DecodificarColor(param_3);
                    float3 param_5 = normal;
                    float3 param_6 = rayDir;
                    float param_7 = distanciaRecorrida;
                    float param_8 = maxRenderDistance;
                    float param_9 = gamma;
                    float4 _489 = ShadeHit(param_4, param_5, param_6, param_7, param_8, param_9);
                    colorFinal = _489;
                    hit = true;
                }
                else
                {
                    float3 emptyMinWorld = posCajaMin * voxelScale;
                    float3 emptyMaxWorld = boxMax * voxelScale;
                    float3 param_10 = rayPos;
                    float3 param_11 = rayDir;
                    float3 param_12 = emptyMinWorld;
                    float3 param_13 = emptyMaxWorld;
                    float tExit = AABBExitT(param_10, param_11, param_12, param_13);
                    rayPos += (rayDir * fast::max(tExit + epsilon, epsilon));
                    avanzo = true;
                }
                break;
            }
            float halfSize = tamanoActual * 0.5;
            float3 centro = posCajaMin + float3(halfSize);
            uint bitX = uint(indexPos.x >= centro.x);
            uint bitY = uint(indexPos.y >= centro.y);
            uint bitZ = uint(indexPos.z >= centro.z);
            uint octante = (bitX | (bitY << uint(1))) | (bitZ << uint(2));
            uint mascaraOctante = 1u << octante;
            uint indiceHijo = childPtr + octante;
            float3 childBoxMin = posCajaMin + float3((bitX != 0u) ? halfSize : 0.0, (bitY != 0u) ? halfSize : 0.0, (bitZ != 0u) ? halfSize : 0.0);
            float3 childBoxMax = childBoxMin + float3(halfSize);
            if ((leafMask & mascaraOctante) != 0u)
            {
                uint param_14 = v_412.nodes[indiceHijo].material;
                float4 hitColor = DecodificarColor(param_14);
                if (hitColor.w > 0.0)
                {
                    float3 param_15 = indexPos;
                    float3 param_16 = childBoxMin;
                    float3 param_17 = childBoxMax;
                    float3 normal_1 = BoxNormal(param_15, param_16, param_17);
                    float4 param_18 = hitColor;
                    float3 param_19 = normal_1;
                    float3 param_20 = rayDir;
                    float param_21 = distanciaRecorrida;
                    float param_22 = maxRenderDistance;
                    float param_23 = gamma;
                    float4 _624 = ShadeHit(param_18, param_19, param_20, param_21, param_22, param_23);
                    colorFinal = _624;
                    hit = true;
                    break;
                }
            }
            if ((validMask & mascaraOctante) != 0u)
            {
                nodoActual = indiceHijo;
                tamanoActual = halfSize;
                posCajaMin = childBoxMin;
            }
            else
            {
                float3 emptyBoxMinWorld = childBoxMin * voxelScale;
                float3 emptyBoxMaxWorld = childBoxMax * voxelScale;
                float3 param_24 = rayPos;
                float3 param_25 = rayDir;
                float3 param_26 = emptyBoxMinWorld;
                float3 param_27 = emptyBoxMaxWorld;
                float tExit_1 = AABBExitT(param_24, param_25, param_26, param_27);
                rayPos += (rayDir * fast::max(tExit_1 + epsilon, epsilon));
                avanzo = true;
                break;
            }
        }
        if (hit)
        {
            break;
        }
        if (!avanzo)
        {
            rayPos += (rayDir * epsilon);
        }
    }
    return colorFinal;
}

kernel void compute_main(const device NodeBuffer& v_412 [[buffer(0)]], constant FrameBlock& _721 [[buffer(1)]], texture2d<float, access::write> outputTexture [[texture(0)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]])
{
    int2 gridPos = int2(gl_GlobalInvocationID.xy);
    int2 dims = int2(outputTexture.get_width(), outputTexture.get_height());
    bool _704 = gridPos.x >= dims.x;
    bool _713;
    if (!_704)
    {
        _713 = gridPos.y >= dims.y;
    }
    else
    {
        _713 = _704;
    }
    if (_713)
    {
        return;
    }
    float3 cameraPos = _721.u_Frame.CameraPos.xyz;
    float3 cameraDir = _721.u_Frame.CameraDir.xyz;
    float3 cameraUp = _721.u_Frame.CameraUp.xyz;
    float3 cameraRight = _721.u_Frame.CameraRight.xyz;
    float fov = _721.u_Frame.Fov;
    float worldSize = _721.u_Frame.WorldSize;
    int maxLevels = int(_721.u_Frame.MaxLevels);
    float voxelScale = _721.u_Frame.VoxelScale;
    float lodPixelThreshold = _721.u_Frame.LodPixelThreshold;
    float lodCoverageThreshold = _721.u_Frame.LodCoverageThreshold;
    float4 colorAccum = float4(0.0);
    for (int sy = 0; sy < 2; sy++)
    {
        for (int sx = 0; sx < 2; sx++)
        {
            float2 subOffset = ((float2(float(sx), float(sy)) + float2(0.5)) / float2(2.0)) - float2(0.5);
            float2 uv = (((float2(gridPos) + subOffset) / float2(dims)) * 2.0) - float2(1.0);
            uv.x *= (float(dims.x) / float(dims.y));
            float3 rayDir = fast::normalize((cameraDir + ((cameraRight * uv.x) * fov)) + ((cameraUp * uv.y) * fov));
            float3 param = cameraPos;
            float3 param_1 = rayDir;
            float param_2 = worldSize;
            int param_3 = maxLevels;
            float param_4 = voxelScale;
            float param_5 = lodPixelThreshold;
            float param_6 = lodCoverageThreshold;
            colorAccum += TraceRay(param, param_1, param_2, param_3, param_4, param_5, param_6, v_412);
        }
    }
    float4 colorFinal = colorAccum / float4(4.0);
    outputTexture.write(colorFinal, uint2(gridPos));
}

