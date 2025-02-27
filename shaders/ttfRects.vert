#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

layout(set = 1, binding = 0) uniform UniformBufferObject {
  vec2 screenSize;
};

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outNormal;

void main() {
  vec4 outPos = vec4(inPos.xy / (0.5 * screenSize), inPos.z, 1.0);
  outPos.x = outPos.x - 1.0;
  outPos.y = outPos.y + 1.0;
  outUv = inUv;
  outNormal = inNormal;
  gl_Position = outPos;
}