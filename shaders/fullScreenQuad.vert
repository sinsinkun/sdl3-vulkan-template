#version 450

vec2 corners[6] = vec2[](
  vec2(-1.0, 1.0),
  vec2( 1.0, 1.0),
  vec2( 1.0,-1.0),
  vec2(-1.0, 1.0),
  vec2( 1.0,-1.0),
  vec2(-1.0,-1.0)
);

void main() {
  int i = gl_VertexIndex;
  gl_Position = vec4(corners[i], 0.0, 1.0);
}