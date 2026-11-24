#version 460
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct SVONodeGPU
{
    uint descriptor;
    uint childIndex;
    uint material;
};

struct FrameData
{
    vec4 CameraPos;
    vec4 CameraDir;
    vec4 CameraUp;
    vec4 CameraRight;
    float Fov;
    float WorldSize;
    float MaxLevels;
    float VoxelScale;
    float LodPixelThreshold;
    float LodCoverageThreshold;
    float _Pad0;
    float _Pad1;
};

layout(binding = 0, std430) readonly buffer NodeBuffer
{
    SVONodeGPU nodes[];
} _412;

layout(binding = 1, std140) uniform FrameBlock
{
    FrameData u_Frame;
} _721;

layout(binding = 0, rgba8) uniform writeonly image2D outputTexture;

vec3 BoxNormal(vec3 indexPos, vec3 boxMin, vec3 boxMax)
{
    vec3 distToMin = indexPos - boxMin;
    vec3 distToMax = boxMax - indexPos;
    vec3 normal = vec3(-1.0, 0.0, 0.0);
    float minDist = distToMin.x;
    if (distToMin.y < minDist)
    {
        minDist = distToMin.y;
        normal = vec3(0.0, -1.0, 0.0);
    }
    if (distToMin.z < minDist)
    {
        minDist = distToMin.z;
        normal = vec3(0.0, 0.0, -1.0);
    }
    if (distToMax.x < minDist)
    {
        minDist = distToMax.x;
        normal = vec3(1.0, 0.0, 0.0);
    }
    if (distToMax.y < minDist)
    {
        minDist = distToMax.y;
        normal = vec3(0.0, 1.0, 0.0);
    }
    if (distToMax.z < minDist)
    {
        minDist = distToMax.z;
        normal = vec3(0.0, 0.0, 1.0);
    }
    return normal;
}

vec4 DecodificarColor(uint material)
{
    float r = float(material & 255u) / 255.0;
    float g = float((material >> uint(8)) & 255u) / 255.0;
    float b = float((material >> uint(16)) & 255u) / 255.0;
    float a = float((material >> uint(24)) & 255u) / 255.0;
    if (a == 0.0)
    {
        return vec4(r, g, b, a);
    }
    return vec4(r, g, b, 1.0);
}

vec4 ShadeHit(inout vec4 baseColor, vec3 normal, vec3 rayDir, float distanciaRecorrida, float maxRenderDistance, float gamma)
{
    float ambient = 0.119999997317790985107421875;
    float diffuse = max(dot(normal, -rayDir), 0.0);
    float lighting = ambient + ((1.0 - ambient) * diffuse);
    vec4 _287 = baseColor;
    vec3 _289 = _287.xyz * lighting;
    baseColor.x = _289.x;
    baseColor.y = _289.y;
    baseColor.z = _289.z;
    float atenuacion = clamp(1.0 - (distanciaRecorrida / maxRenderDistance), 0.0, 1.0);
    vec4 _303 = baseColor;
    vec3 _305 = _303.xyz * atenuacion;
    baseColor.x = _305.x;
    baseColor.y = _305.y;
    baseColor.z = _305.z;
    vec4 _312 = baseColor;
    vec3 _317 = pow(_312.xyz, vec3(1.0 / gamma));
    baseColor.x = _317.x;
    baseColor.y = _317.y;
    baseColor.z = _317.z;
    return baseColor;
}

float AABBExitT(vec3 origin, vec3 dir, vec3 boxMin, vec3 boxMax)
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
    return min(tx, min(ty, tz));
}

vec4 TraceRay(vec3 rayOrigin, vec3 rayDir, float worldSizeIndex, int maxLevels, float voxelScale, float lodPixelThreshold, float lodCoverageThreshold)
{
    float gamma = 2.2000000476837158203125;
    float worldSizeWorld = worldSizeIndex * voxelScale;
    float maxRenderDistance = 40.0;
    float epsilon = max(voxelScale * 0.00999999977648258209228515625, 0.0005000000237487256526947021484375);
    vec3 rayPos = rayOrigin;
    vec4 colorFinal = vec4(0.0500000007450580596923828125, 0.0500000007450580596923828125, 0.07999999821186065673828125, 1.0);
    bool hit = false;
    for (int paso = 0; paso < 128; paso++)
    {
        bool _364 = any(lessThan(rayPos, vec3(0.0)));
        bool _373;
        if (!_364)
        {
            _373 = any(greaterThan(rayPos, vec3(worldSizeWorld)));
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
        vec3 indexPos = rayPos / vec3(voxelScale);
        uint nodoActual = 0u;
        vec3 posCajaMin = vec3(0.0);
        float tamanoActual = worldSizeIndex;
        bool avanzo = false;
        for (int nivel = 0; nivel < maxLevels; nivel++)
        {
            uint desc = _412.nodes[nodoActual].descriptor;
            uint validMask = desc & 255u;
            uint leafMask = (desc >> uint(8)) & 255u;
            uint coverage = (desc >> uint(16)) & 255u;
            uint childPtr = _412.nodes[nodoActual].childIndex;
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
                vec3 boxMax = posCajaMin + vec3(tamanoActual);
                if (float(coverage) > (lodCoverageThreshold * 255.0))
                {
                    vec3 param = indexPos;
                    vec3 param_1 = posCajaMin;
                    vec3 param_2 = boxMax;
                    vec3 normal = BoxNormal(param, param_1, param_2);
                    uint param_3 = _412.nodes[nodoActual].material;
                    vec4 param_4 = DecodificarColor(param_3);
                    vec3 param_5 = normal;
                    vec3 param_6 = rayDir;
                    float param_7 = distanciaRecorrida;
                    float param_8 = maxRenderDistance;
                    float param_9 = gamma;
                    vec4 _489 = ShadeHit(param_4, param_5, param_6, param_7, param_8, param_9);
                    colorFinal = _489;
                    hit = true;
                }
                else
                {
                    vec3 emptyMinWorld = posCajaMin * voxelScale;
                    vec3 emptyMaxWorld = boxMax * voxelScale;
                    vec3 param_10 = rayPos;
                    vec3 param_11 = rayDir;
                    vec3 param_12 = emptyMinWorld;
                    vec3 param_13 = emptyMaxWorld;
                    float tExit = AABBExitT(param_10, param_11, param_12, param_13);
                    rayPos += (rayDir * max(tExit + epsilon, epsilon));
                    avanzo = true;
                }
                break;
            }
            float halfSize = tamanoActual * 0.5;
            vec3 centro = posCajaMin + vec3(halfSize);
            uint bitX = uint(indexPos.x >= centro.x);
            uint bitY = uint(indexPos.y >= centro.y);
            uint bitZ = uint(indexPos.z >= centro.z);
            uint octante = (bitX | (bitY << uint(1))) | (bitZ << uint(2));
            uint mascaraOctante = 1u << octante;
            uint indiceHijo = childPtr + octante;
            vec3 childBoxMin = posCajaMin + vec3((bitX != 0u) ? halfSize : 0.0, (bitY != 0u) ? halfSize : 0.0, (bitZ != 0u) ? halfSize : 0.0);
            vec3 childBoxMax = childBoxMin + vec3(halfSize);
            if ((leafMask & mascaraOctante) != 0u)
            {
                uint param_14 = _412.nodes[indiceHijo].material;
                vec4 hitColor = DecodificarColor(param_14);
                if (hitColor.w > 0.0)
                {
                    vec3 param_15 = indexPos;
                    vec3 param_16 = childBoxMin;
                    vec3 param_17 = childBoxMax;
                    vec3 normal_1 = BoxNormal(param_15, param_16, param_17);
                    vec4 param_18 = hitColor;
                    vec3 param_19 = normal_1;
                    vec3 param_20 = rayDir;
                    float param_21 = distanciaRecorrida;
                    float param_22 = maxRenderDistance;
                    float param_23 = gamma;
                    vec4 _624 = ShadeHit(param_18, param_19, param_20, param_21, param_22, param_23);
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
                vec3 emptyBoxMinWorld = childBoxMin * voxelScale;
                vec3 emptyBoxMaxWorld = childBoxMax * voxelScale;
                vec3 param_24 = rayPos;
                vec3 param_25 = rayDir;
                vec3 param_26 = emptyBoxMinWorld;
                vec3 param_27 = emptyBoxMaxWorld;
                float tExit_1 = AABBExitT(param_24, param_25, param_26, param_27);
                rayPos += (rayDir * max(tExit_1 + epsilon, epsilon));
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

void main()
{
    ivec2 gridPos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(outputTexture);
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
    vec3 cameraPos = _721.u_Frame.CameraPos.xyz;
    vec3 cameraDir = _721.u_Frame.CameraDir.xyz;
    vec3 cameraUp = _721.u_Frame.CameraUp.xyz;
    vec3 cameraRight = _721.u_Frame.CameraRight.xyz;
    float fov = _721.u_Frame.Fov;
    float worldSize = _721.u_Frame.WorldSize;
    int maxLevels = int(_721.u_Frame.MaxLevels);
    float voxelScale = _721.u_Frame.VoxelScale;
    float lodPixelThreshold = _721.u_Frame.LodPixelThreshold;
    float lodCoverageThreshold = _721.u_Frame.LodCoverageThreshold;
    vec4 colorAccum = vec4(0.0);
    for (int sy = 0; sy < 2; sy++)
    {
        for (int sx = 0; sx < 2; sx++)
        {
            vec2 subOffset = ((vec2(float(sx), float(sy)) + vec2(0.5)) / vec2(2.0)) - vec2(0.5);
            vec2 uv = (((vec2(gridPos) + subOffset) / vec2(dims)) * 2.0) - vec2(1.0);
            uv.x *= (float(dims.x) / float(dims.y));
            vec3 rayDir = normalize((cameraDir + ((cameraRight * uv.x) * fov)) + ((cameraUp * uv.y) * fov));
            vec3 param = cameraPos;
            vec3 param_1 = rayDir;
            float param_2 = worldSize;
            int param_3 = maxLevels;
            float param_4 = voxelScale;
            float param_5 = lodPixelThreshold;
            float param_6 = lodCoverageThreshold;
            colorAccum += TraceRay(param, param_1, param_2, param_3, param_4, param_5, param_6);
        }
    }
    vec4 colorFinal = colorAccum / vec4(4.0);
    imageStore(outputTexture, gridPos, colorFinal);
}

