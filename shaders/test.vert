#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outNormal;

void main() {
    gl_Position = vec4(inPos, 1.0);
    outUv = inUv;
    outNormal = inNormal;
}