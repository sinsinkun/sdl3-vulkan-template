#version 450

layout(set = 2, binding = 0) uniform sampler2D texture0;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) out vec3 pos;

layout(set = 3, binding = 0) uniform UniformBufferObject {
  vec4 albedo;
};

layout(location = 0) out vec4 outColor;

void main() {
  // base color
  vec4 tx = texture(texture0, uv);
  float useTx = step(0.001, tx.a);
  vec4 baseColor = mix(albedo, tx, useTx);

  // todo: diffuse
  // todo: specular

  outColor = baseColor;
}