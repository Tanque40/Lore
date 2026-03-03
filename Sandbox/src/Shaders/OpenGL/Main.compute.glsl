#version 460 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform image2D u_OutputImage;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imgSize = imageSize(u_OutputImage);

    if (texelCoord.x >= imgSize.x || texelCoord.y >= imgSize.y)
        return;

    vec2 uv = vec2(texelCoord) / vec2(imgSize);
    vec4 color = vec4(uv.x, uv.y, 0.5, 1.0);
    imageStore(u_OutputImage, texelCoord, color);
}
