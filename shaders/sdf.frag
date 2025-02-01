#version 450

struct SDFObject {
  uint objType;
  float radius;
  vec2 center; // 1st quad
  vec2 v2;
  vec2 v3; // 2nd quad
  float cornerRadius;
  float rotation;
  float thickness; // 3rd quad
  vec4 color; // 4th quad
};

layout(set=2, binding=0) buffer readonly SDFStorage {
  SDFObject sdfObjects[];
};

layout(set=3, binding=0) uniform SysData {
  vec2 screenSize;
  vec2 mousePos;
  int lightDist;
  vec2 lightPos;
  vec4 lightColor;
};

layout(location=0) out vec4 outColor;

// ----------------------------------------- //
// ------------- SDF FUNCTIONS ------------- //
// ----------------------------------------- //
float sdCircle(vec2 p, vec2 c, float r) {
  return length(p - c) - r;
}

// round corners
float opRound(float sd, float r) {
  return sd - r;
}

// hollow center
float opOnion(float sd, float r) {
  return abs(sd) - r;
}

struct SdfOut {
  float dist;
  vec4 color;
};

SdfOut calculateSdf(vec2 p, float maxDist) {
  float dist;
  vec4 clr = vec4(0.0);
  for (uint i = 0; i < sdfObjects.length(); i++) {
    SDFObject obj = sdfObjects[i];
    float d = maxDist;
    if (obj.objType == 1) { // circle
      d = sdCircle(p, obj.center, obj.radius);
    }
    if (obj.cornerRadius > 0.0) {
      d = opRound(d, obj.cornerRadius);
    }
    if (obj.thickness > 0.0) {
      d = opOnion(d, obj.thickness);
    }
    dist = min(d, dist);
    float intensity = smoothstep(1.0, -1.0, d);
    clr = mix(clr, obj.color, intensity * obj.color.a);
  }
  SdfOut sdf;
  sdf.dist = dist;
  sdf.color = clr;
  return sdf;
}

// ----------------------------------------- //
// ----------- SHADER DEFINITION ----------- //
// ----------------------------------------- //
void main() {
  vec2 p = gl_FragCoord.xy;
  SdfOut sdf = calculateSdf(p, 10000.0);
  outColor = sdf.color;
}