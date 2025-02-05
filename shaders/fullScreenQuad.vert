#version 450

vec2 corners[6] = vec2[](
  vec2(-1.0, 1.0),
  vec2( 1.0, 1.0),
  vec2( 1.0,-1.0),
  vec2(-1.0, 1.0),
  vec2( 1.0,-1.0),
  vec2(-1.0,-1.0)
);

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outNormal;

void main() {
  int i = gl_VertexIndex;
  vec2 uv = (corners[i] + vec2(1.0, -1.0)) / 2.0;
  outUv = vec2(uv.x, -1.0 * uv.y);
  outNormal = vec3(0.0, 0.0, 1.0);
  gl_Position = vec4(corners[i], 0.0, 1.0);
}