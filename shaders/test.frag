#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

// set 2: sampled textures -> storage textures -> storge buffers
// set 3: uniform buffers

layout(location = 0) out vec4 outColor;

void main() {
    vec3 pos = gl_FragCoord.xyz;
    outColor = vec4(uv, 0.0, 1.0);
}