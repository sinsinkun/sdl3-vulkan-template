#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

layout(binding = 0) uniform UniformBufferObject {
    mat4 translateMat;
} ubo;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outNormal;

void main() {
    vec4 outPos = vec4(inPos, 1.0);
    outUv = inUv;
    outNormal = inNormal;
    gl_Position = outPos;
}