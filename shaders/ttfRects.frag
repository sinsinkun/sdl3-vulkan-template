#version 450

layout(set = 2, binding = 0) uniform sampler2D texture0;

layout(set = 3, binding = 0) uniform UniformBufferObject {
  vec4 color;
};

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 text = texture(texture0, uv);
  outColor = text * color;
}