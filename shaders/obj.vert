#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

layout(set = 1, binding = 0) uniform UniformBufferObject {
  mat4x4 model;
  mat4x4 view;
  mat4x4 proj;
};

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outPos;

void main() {
  mat4x4 mvp = proj * view * model;
  vec4 outP = model * vec4(inPos, 1.0);
  // note: non-uniform scaling on the object will break this normal calculation, 
  // in which case we need mat3(transpose(inverse(model))) * inNormal,
  // but inverse is expensive to calculate so it should be passed in via the UBO
  vec4 outN = model * vec4(inNormal, 0.0);
  vec4 outFrag = mvp * vec4(inPos, 1.0);
  outUv = inUv;
  outNormal = outN.xyz;
  outPos = outP.xyz;
  gl_Position = outFrag;
}