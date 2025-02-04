#version 450

layout(set = 2, binding = 0, r8ui) uniform readonly uimage2D texture0;

layout(set = 3, binding = 0) uniform UniformData {
  vec2 screenSize;
};

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
  ivec2 pos = ivec2(gl_FragCoord.xy);
  vec4 tx = imageLoad(texture0, pos);
  outColor = vec4(tx.rrr, 0.5);
}