#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 pos = gl_FragCoord.xyz;
    outColor = vec4(uv, 0.0, 1.0);
}