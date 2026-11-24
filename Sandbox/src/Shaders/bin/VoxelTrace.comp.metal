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

kernel void compute_main(const device NodeBuffer& _412 [[buffer(0)]], constant FrameBlock& _721 [[buffer(1)]], texture2d<float, access::write> outputTexture [[texture(0)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]])
{
    do
    {
        int2 _693 = int2(gl_GlobalInvocationID.xy);
        int2 _699 = int2(outputTexture.get_width(), outputTexture.get_height());
        int _703 = _699.x;
        bool _704 = _693.x >= _703;
        bool _713;
        if (!_704)
        {
            _713 = _693.y >= _699.y;
        }
        else
        {
            _713 = _704;
        }
        if (_713)
        {
            break;
        }
        int _752 = int(_721.u_Frame.MaxLevels);
        float4 _1845;
        _1845 = float4(0.0);
        float4 _2019;
        for (int _1844 = 0; _1844 < 2; _1845 = _2019, _1844++)
        {
            _2019 = _1845;
            float4 _852;
            for (int _1846 = 0; _1846 < 2; _2019 = _852, _1846++)
            {
                float2 _807 = (((float2(_693) + (((float2(float(_1846), float(_1844)) + float2(0.5)) * float2(0.5)) - float2(0.5))) / float2(_699)) * 2.0) - float2(1.0);
                float3 _835 = fast::normalize((_721.u_Frame.CameraDir.xyz + ((_721.u_Frame.CameraRight.xyz * (_807.x * (float(_703) / float(_699.y)))) * _721.u_Frame.Fov)) + ((_721.u_Frame.CameraUp.xyz * _807.y) * _721.u_Frame.Fov));
                float _948 = _721.u_Frame.WorldSize * _721.u_Frame.VoxelScale;
                float _951 = fast::max(_721.u_Frame.VoxelScale * 0.00999999977648258209228515625, 0.0005000000237487256526947021484375);
                float3 _1849;
                float4 _1996;
                _1849 = _721.u_Frame.CameraPos.xyz;
                _1996 = float4(0.0500000007450580596923828125, 0.0500000007450580596923828125, 0.07999999821186065673828125, 1.0);
                bool _1914;
                float4 _1995;
                float4 _1998;
                float3 _2117;
                int _1848 = 0;
                bool _1917 = false;
                for (;;)
                {
                    if (_1848 < 128)
                    {
                        bool _960 = any(_1849 < float3(0.0));
                        bool _969;
                        if (!_960)
                        {
                            _969 = any(_1849 > float3(_948));
                        }
                        else
                        {
                            _969 = _960;
                        }
                        if (_969)
                        {
                            _1995 = _1996;
                            break;
                        }
                        float _975 = length(_1849 - _721.u_Frame.CameraPos.xyz);
                        if (_975 > 40.0)
                        {
                            _1995 = _1996;
                            break;
                        }
                        float3 _984 = _1849 / float3(_721.u_Frame.VoxelScale);
                        float3 _1856;
                        _1856 = float3(0.0);
                        float _1071;
                        uint _1105;
                        float3 _1120;
                        bool _1934;
                        float3 _1951;
                        int _1851 = 0;
                        uint _1852 = 0u;
                        float _1853 = _721.u_Frame.WorldSize;
                        for (;;)
                        {
                            if (_1851 < _752)
                            {
                                bool _1013 = (_1851 > 0) && (_721.u_Frame.LodPixelThreshold > 0.0);
                                bool _1021;
                                if (_1013)
                                {
                                    _1021 = (_1853 * _721.u_Frame.VoxelScale) < (_975 * _721.u_Frame.LodPixelThreshold);
                                }
                                else
                                {
                                    _1021 = _1013;
                                }
                                if (_1021)
                                {
                                    float3 _1026 = _1856 + float3(_1853);
                                    bool _1031 = float((_412.nodes[_1852].descriptor >> uint(16)) & 255u) > (_721.u_Frame.LodCoverageThreshold * 255.0);
                                    float3 _1952;
                                    float4 _2004;
                                    if (_1031)
                                    {
                                        float3 _1210 = _984 - _1856;
                                        float3 _1213 = _1026 - _984;
                                        float _1215 = _1210.x;
                                        float _1217 = _1210.y;
                                        bool _1219 = _1217 < _1215;
                                        float _2137 = _1219 ? _1217 : _1215;
                                        float _1225 = _1210.z;
                                        bool _1227 = _1225 < _2137;
                                        float _2140 = _1227 ? _1225 : _2137;
                                        float _1233 = _1213.x;
                                        bool _1235 = _1233 < _2140;
                                        float _2143 = _1235 ? _1233 : _2140;
                                        float _1241 = _1213.y;
                                        bool _1243 = _1241 < _2143;
                                        float4 _1913;
                                        do
                                        {
                                            float _1269 = float(_412.nodes[_1852].material & 255u) * 0.0039215688593685626983642578125;
                                            float _1274 = float((_412.nodes[_1852].material >> uint(8)) & 255u) * 0.0039215688593685626983642578125;
                                            float _1279 = float((_412.nodes[_1852].material >> uint(16)) & 255u) * 0.0039215688593685626983642578125;
                                            float _1284 = float((_412.nodes[_1852].material >> uint(24)) & 255u) * 0.0039215688593685626983642578125;
                                            if (_1284 == 0.0)
                                            {
                                                _1913 = float4(_1269, _1274, _1279, _1284);
                                                break;
                                            }
                                            _1913 = float4(_1269, _1274, _1279, 1.0);
                                            break;
                                        } while(false);
                                        float3 _1320 = _1913.xyz * (0.119999997317790985107421875 + (0.87999999523162841796875 * fast::max(dot(select(select(select(select(select(float3(-1.0, 0.0, 0.0), float3(0.0, -1.0, 0.0), bool3(_1219)), float3(0.0, 0.0, -1.0), bool3(_1227)), float3(1.0, 0.0, 0.0), bool3(_1235)), float3(0.0, 1.0, 0.0), bool3(_1243)), float3(0.0, 0.0, 1.0), bool3(_1213.z < (_1243 ? _1241 : _2143))), -_835), 0.0)));
                                        float4 _1743 = _1913;
                                        _1743.x = _1320.x;
                                        _1743.y = _1320.y;
                                        _1743.z = _1320.z;
                                        float3 _1335 = _1743.xyz * fast::clamp(1.0 - (_975 * 0.02500000037252902984619140625), 0.0, 1.0);
                                        float4 _1749 = _1743;
                                        _1749.x = _1335.x;
                                        _1749.y = _1335.y;
                                        _1749.z = _1335.z;
                                        float3 _1347 = pow(_1749.xyz, float3(0.454545438289642333984375));
                                        float4 _1755 = _1749;
                                        _1755.x = _1347.x;
                                        _1755.y = _1347.y;
                                        _1755.z = _1347.z;
                                        _2004 = _1755;
                                        _1952 = _1849;
                                    }
                                    else
                                    {
                                        float3 _1050 = _1856 * _721.u_Frame.VoxelScale;
                                        float3 _1053 = _1026 * _721.u_Frame.VoxelScale;
                                        float _1367 = _835.x;
                                        float _1887;
                                        if (_1367 > 0.0)
                                        {
                                            _1887 = (_1053.x - _1849.x) / _1367;
                                        }
                                        else
                                        {
                                            float _1886;
                                            if (_1367 < 0.0)
                                            {
                                                _1886 = (_1050.x - _1849.x) / _1367;
                                            }
                                            else
                                            {
                                                _1886 = 1000000015047466219876688855040.0;
                                            }
                                            _1887 = _1886;
                                        }
                                        float _1397 = _835.y;
                                        float _1889;
                                        if (_1397 > 0.0)
                                        {
                                            _1889 = (_1053.y - _1849.y) / _1397;
                                        }
                                        else
                                        {
                                            float _1888;
                                            if (_1397 < 0.0)
                                            {
                                                _1888 = (_1050.y - _1849.y) / _1397;
                                            }
                                            else
                                            {
                                                _1888 = 1000000015047466219876688855040.0;
                                            }
                                            _1889 = _1888;
                                        }
                                        float _1427 = _835.z;
                                        float _1891;
                                        if (_1427 > 0.0)
                                        {
                                            _1891 = (_1053.z - _1849.z) / _1427;
                                        }
                                        else
                                        {
                                            float _1890;
                                            if (_1427 < 0.0)
                                            {
                                                _1890 = (_1050.z - _1849.z) / _1427;
                                            }
                                            else
                                            {
                                                _1890 = 1000000015047466219876688855040.0;
                                            }
                                            _1891 = _1890;
                                        }
                                        _2004 = _1996;
                                        _1952 = _1849 + (_835 * fast::max(fast::min(_1887, fast::min(_1889, _1891)) + _951, _951));
                                    }
                                    _1998 = _2004;
                                    _1951 = _1952;
                                    _1934 = _1031 ? false : true;
                                    _1914 = _1031 ? true : _1917;
                                    break;
                                }
                                _1071 = _1853 * 0.5;
                                float3 _1074 = float3(_1071);
                                float3 _1075 = _1856 + _1074;
                                uint _1081 = uint(_984.x >= _1075.x);
                                uint _1087 = uint(_984.y >= _1075.y);
                                uint _1093 = uint(_984.z >= _1075.z);
                                uint _1100 = (_1081 | (_1087 << uint(1))) | (_1093 << uint(2));
                                uint _1102 = 1u << _1100;
                                _1105 = _412.nodes[_1852].childIndex + _1100;
                                _1120 = _1856 + float3((_1081 != 0u) ? _1071 : 0.0, (_1087 != 0u) ? _1071 : 0.0, (_1093 != 0u) ? _1071 : 0.0);
                                float3 _1124 = _1120 + _1074;
                                if ((((_412.nodes[_1852].descriptor >> uint(8)) & 255u) & _1102) != 0u)
                                {
                                    float4 _1857;
                                    do
                                    {
                                        float _1473 = float(_412.nodes[_1105].material & 255u) * 0.0039215688593685626983642578125;
                                        float _1478 = float((_412.nodes[_1105].material >> uint(8)) & 255u) * 0.0039215688593685626983642578125;
                                        float _1483 = float((_412.nodes[_1105].material >> uint(16)) & 255u) * 0.0039215688593685626983642578125;
                                        float _1488 = float((_412.nodes[_1105].material >> uint(24)) & 255u) * 0.0039215688593685626983642578125;
                                        if (_1488 == 0.0)
                                        {
                                            _1857 = float4(_1473, _1478, _1483, _1488);
                                            break;
                                        }
                                        _1857 = float4(_1473, _1478, _1483, 1.0);
                                        break;
                                    } while(false);
                                    if (_1857.w > 0.0)
                                    {
                                        float3 _1512 = _984 - _1120;
                                        float3 _1515 = _1124 - _984;
                                        float _1517 = _1512.x;
                                        float _1519 = _1512.y;
                                        bool _1521 = _1519 < _1517;
                                        float _2153 = _1521 ? _1519 : _1517;
                                        float _1527 = _1512.z;
                                        bool _1529 = _1527 < _2153;
                                        float _2156 = _1529 ? _1527 : _2153;
                                        float _1535 = _1515.x;
                                        bool _1537 = _1535 < _2156;
                                        float _2159 = _1537 ? _1535 : _2156;
                                        float _1543 = _1515.y;
                                        bool _1545 = _1543 < _2159;
                                        float3 _1579 = _1857.xyz * (0.119999997317790985107421875 + (0.87999999523162841796875 * fast::max(dot(select(select(select(select(select(float3(-1.0, 0.0, 0.0), float3(0.0, -1.0, 0.0), bool3(_1521)), float3(0.0, 0.0, -1.0), bool3(_1529)), float3(1.0, 0.0, 0.0), bool3(_1537)), float3(0.0, 1.0, 0.0), bool3(_1545)), float3(0.0, 0.0, 1.0), bool3(_1515.z < (_1545 ? _1543 : _2159))), -_835), 0.0)));
                                        float4 _1803 = _1857;
                                        _1803.x = _1579.x;
                                        _1803.y = _1579.y;
                                        _1803.z = _1579.z;
                                        float3 _1594 = _1803.xyz * fast::clamp(1.0 - (_975 * 0.02500000037252902984619140625), 0.0, 1.0);
                                        float4 _1809 = _1803;
                                        _1809.x = _1594.x;
                                        _1809.y = _1594.y;
                                        _1809.z = _1594.z;
                                        float3 _1606 = pow(_1809.xyz, float3(0.454545438289642333984375));
                                        float4 _1815 = _1809;
                                        _1815.x = _1606.x;
                                        _1815.y = _1606.y;
                                        _1815.z = _1606.z;
                                        _1998 = _1815;
                                        _1951 = _1849;
                                        _1934 = false;
                                        _1914 = true;
                                        break;
                                    }
                                }
                                if (!(((_412.nodes[_1852].descriptor & 255u) & _1102) != 0u))
                                {
                                    float3 _1162 = _1120 * _721.u_Frame.VoxelScale;
                                    float3 _1165 = _1124 * _721.u_Frame.VoxelScale;
                                    float _1626 = _835.x;
                                    float _1863;
                                    if (_1626 > 0.0)
                                    {
                                        _1863 = (_1165.x - _1849.x) / _1626;
                                    }
                                    else
                                    {
                                        float _1862;
                                        if (_1626 < 0.0)
                                        {
                                            _1862 = (_1162.x - _1849.x) / _1626;
                                        }
                                        else
                                        {
                                            _1862 = 1000000015047466219876688855040.0;
                                        }
                                        _1863 = _1862;
                                    }
                                    float _1656 = _835.y;
                                    float _1865;
                                    if (_1656 > 0.0)
                                    {
                                        _1865 = (_1165.y - _1849.y) / _1656;
                                    }
                                    else
                                    {
                                        float _1864;
                                        if (_1656 < 0.0)
                                        {
                                            _1864 = (_1162.y - _1849.y) / _1656;
                                        }
                                        else
                                        {
                                            _1864 = 1000000015047466219876688855040.0;
                                        }
                                        _1865 = _1864;
                                    }
                                    float _1686 = _835.z;
                                    float _1867;
                                    if (_1686 > 0.0)
                                    {
                                        _1867 = (_1165.z - _1849.z) / _1686;
                                    }
                                    else
                                    {
                                        float _1866;
                                        if (_1686 < 0.0)
                                        {
                                            _1866 = (_1162.z - _1849.z) / _1686;
                                        }
                                        else
                                        {
                                            _1866 = 1000000015047466219876688855040.0;
                                        }
                                        _1867 = _1866;
                                    }
                                    _1998 = _1996;
                                    _1951 = _1849 + (_835 * fast::max(fast::min(_1863, fast::min(_1865, _1867)) + _951, _951));
                                    _1934 = true;
                                    _1914 = _1917;
                                    break;
                                }
                                _1856 = _1120;
                                _1853 = _1071;
                                _1852 = _1105;
                                _1851++;
                                continue;
                            }
                            else
                            {
                                _1998 = _1996;
                                _1951 = _1849;
                                _1934 = false;
                                _1914 = _1917;
                                break;
                            }
                        }
                        if (_1914)
                        {
                            _1995 = _1998;
                            break;
                        }
                        if (!_1934)
                        {
                            _2117 = _1951 + (_835 * _951);
                        }
                        else
                        {
                            _2117 = _1951;
                        }
                        _1917 = _1914;
                        _1849 = _2117;
                        _1848++;
                        _1996 = _1998;
                        continue;
                    }
                    else
                    {
                        _1995 = _1996;
                        break;
                    }
                }
                _852 = _2019 + _1995;
            }
        }
        outputTexture.write(_1845 * float4(0.25), uint2(_693));
        break;
    } while(false);
}

