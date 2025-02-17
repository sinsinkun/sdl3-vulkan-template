#version 450

layout(set = 2, binding = 0) uniform sampler2D texture0;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 tx = texture(texture0, uv);
  float useTx = step(0.001, tx.a);
  outColor = mix(vec4(uv, normal.yz), tx, useTx);
}