#version 450

layout(set = 2, binding = 0) uniform sampler2D texture0;

layout(set = 3, binding = 0) uniform UniformData {
  vec2 screenSize;
};

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
  vec3 pos = gl_FragCoord.xyz;
  outColor = vec4(uv, 0.0, 0.5);
}