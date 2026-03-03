#include <metal_stdlib>
using namespace metal;

kernel void compute_main(
    texture2d<float, access::write> output [[texture(0)]],
    uint2 gid [[thread_position_in_grid]]) {

    if (gid.x >= output.get_width() || gid.y >= output.get_height())
        return;

    float2 uv = float2(gid) / float2(output.get_width(), output.get_height());
    float4 color = float4(uv.x, uv.y, 0.5, 1.0);
    output.write(color, gid);
}
