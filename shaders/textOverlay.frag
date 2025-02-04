#version 450

layout(set = 2, binding = 0, r8ui) uniform readonly uimage2D texture0;

layout(set = 3, binding = 0) uniform UniformData {
  vec2 screenSize;
};

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
  ivec2 pos = ivec2(gl_FragCoord.xy);
  float v = imageLoad(texture0, pos).r / 255.0;
  outColor = vec4(1.0, 1.0, 1.0, v);
}