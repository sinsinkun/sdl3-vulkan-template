#version 450

layout(set = 2, binding = 0) uniform sampler2D texture0;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 pos;

layout(set = 3, binding = 0) uniform UniformBufferObject {
  vec4 lightColor;
  vec3 lightPos;
  float ambientIntensity;
  float specularIntensity;
  float shininess;
  vec4 albedo;
  vec3 cameraPos;
};

layout(location = 0) out vec4 outColor;

void main() {
  // base color
  vec4 tx = texture(texture0, uv);
  float useTx = step(0.001, tx.a);
  vec4 baseColor = mix(albedo, tx, useTx);

  // ambience
  vec3 ambient = vec3(ambientIntensity * lightColor.rgb);

  // diffuse
  vec3 n = normalize(normal);
  vec3 ld = normalize(lightPos - pos);
  float diffusion = max(dot(n, ld), 0.0);
  vec3 diffuse = vec3(diffusion * lightColor.rgb);

  // specular
  vec3 vd = normalize(cameraPos - pos);
  vec3 rd = reflect(-ld, n);
  float spec = pow(max(dot(vd, rd), 0.0), shininess);
  vec3 specular = specularIntensity * spec * lightColor.rgb;

  outColor = vec4((ambient + diffuse + specular) * baseColor.xyz, baseColor.a);
}