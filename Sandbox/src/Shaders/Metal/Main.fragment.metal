#include <metal_stdlib>
using namespace metal;

struct FragmentIn {
    float4 position [[position]];
    float4 color;
};

fragment float4 fragment_main(FragmentIn in [[stage_in]]) {
    return in.color;
}
