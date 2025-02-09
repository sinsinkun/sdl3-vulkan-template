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
  vec2 lightPos;
  vec4 lightColor;
  float lightMaxDist;
  uint objCount;
};

layout(location=0) out vec4 outColor;

// ----------------------------------------- //
// ------------- SDF FUNCTIONS ------------- //
// ----------------------------------------- //
float sdCircle(vec2 p, vec2 c, float r) {
  return length(p - c) - r;
}

float sdLine(vec2 p, vec2 a, vec2 b) {
  vec2 pa = p - a;
  vec2 ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
  return length(pa - ba * h);
}

float sdTriangle(vec2 p, vec2 p0, vec2 p1, vec2 p2) {
  vec2 e0 = p1 - p0;
  vec2 v0 = p - p0;
  vec2 d0 = v0 - e0 * clamp(dot(v0,e0)/dot(e0,e0),0.0,1.0);
  float d0d = dot(d0, d0);

  vec2 e1 = p2 - p1;
  vec2 v1 = p - p1;
  vec2 d1 = v1 - e1 * clamp(dot(v1,e1)/dot(e1,e1),0.0,1.0);
  float d1d = dot(d1, d1);

  vec2 e2 = p0 - p2;
  vec2 v2 = p - p2;
  vec2 d2 = v2 - e2 * clamp(dot(v2,e2)/dot(e2,e2),0.0,1.0);
  float d2d = dot(d2, d2);

  float o = e0.x * e2.y - e0.y * e2.x;
  float y0 = o*(v0.x*e0.y - v0.y*e0.x);
  float y1 = o*(v1.x*e1.y - v1.y*e1.x);
  float y2 = o*(v2.x*e2.y - v2.y*e2.x);
  float mind = min(min(d0d, d1d), d2d);
  float miny = min(min(y0, y1), y2);
	return sqrt(mind) * -1.0 * sign(miny);
}

float sdRect(vec2 p, vec2 c, vec2 s) {
  vec2 d = abs(p - c) - s;
  return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
}

float sdRectAngled(vec2 p, vec2 c, vec2 s, float a) {
  p.x = (p.x - c.x) * cos(-a) - (p.y - c.y) * sin(-a) + c.x;
  p.y = (p.y - c.y) * cos(-a) + (p.x - c.x) * sin(-a) + c.y;
  return sdRect(p, c, s);
}

// round corners
float opRound(float sd, float r) {
  return sd - r;
}

// hollow center
float opOnion(float sd, float r) {
  return abs(sd) - r;
}

float opSmoothMerge(float sd1, float sd2, float r) {
  vec2 intsp = min(vec2(sd1 - r, sd2 - r), vec2(0.0));
  return length(intsp) - r;
}

struct SdfOut { float dist; vec4 color; };
SdfOut calculateSdf(vec2 p, float maxDist) {
  float dist = maxDist;
  vec4 clr = vec4(0.0);
  for (uint i = 0; i < objCount; i++) {
    SDFObject obj = sdfObjects[i];
    float d = maxDist;
    if (obj.objType == 1) { // circle
      d = sdCircle(p, obj.center, obj.radius);
    }
    if (obj.objType == 2) { // line
      d = sdLine(p, obj.center, obj.v2);
    }
    if (obj.objType == 3) { // triangle
      d = sdTriangle(p, obj.center, obj.v2, obj.v3);
    }
    if (obj.objType == 4) { // rect
      d = sdRect(p, obj.center, obj.v2);
    }
    if (obj.objType == 5) { // angled rect
      d = sdRectAngled(p, obj.center, obj.v2, obj.rotation);
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

struct RayMarchOut { float dist; float minSdf; };
RayMarchOut rayMarch(vec2 origin, vec2 target, float maxDist) {
  vec2 ndir = normalize(target - origin);
  vec2 p = origin;
  SdfOut sdf = calculateSdf(p, maxDist);
  float rayDist = sdf.dist;
  float minSdf = sdf.dist;
  int iter = 0;
  while (rayDist < maxDist && sdf.dist > 0.999 && iter < 99999) {
    iter += 1;
    p = p + (ndir * sdf.dist);
    sdf = calculateSdf(p, maxDist);
    rayDist += sdf.dist;
    if (sdf.dist < minSdf) {
      minSdf = sdf.dist;
    }
  }

  RayMarchOut rm;
  rm.dist = min(rayDist, maxDist);
  rm.minSdf = minSdf;
  return rm;
}

// ----------------------------------------- //
// ----------- SHADER DEFINITION ----------- //
// ----------------------------------------- //
void main() {
  vec2 p = gl_FragCoord.xy;
  // calculate SDF/D/RM
  SdfOut sdf = calculateSdf(p, 10000.0);
  float distFromLight = distance(p, lightPos);
  RayMarchOut rm = rayMarch(p, lightPos, distFromLight);
  // combine colors
  outColor = sdf.color;
  if (lightMaxDist > 0.01 && sdf.dist > -1.0) {
    vec4 lc = lightColor * smoothstep(lightMaxDist, 0.0, distFromLight);
    outColor += lc;
  }
}