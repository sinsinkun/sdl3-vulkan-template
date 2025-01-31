#include "util.hpp"

using namespace App;

Mat4x4::Mat4x4(
  float i0, float i1, float i2, float i3,
  float i4, float i5, float i6, float i7,
  float i8, float i9, float iA, float iB,
  float iC, float iD, float iE, float iF
) {
  data[0] = i0;
  data[1] = i1;
  data[2] = i2;
  data[3] = i3;
  data[4] = i4;
  data[5] = i5;
  data[6] = i6;
  data[7] = i7;
  data[8] = i8;
  data[9] = i9;
  data[10] = iA;
  data[11] = iB;
  data[12] = iC;
  data[13] = iD;
  data[14] = iE;
  data[15] = iF;
}

Mat4x4::Mat4x4(float values[16]) {
  for (int i=0; i < 16; i++) {
    data[i] = values[i];
  }
};

float* Mat4x4::array() {
  return data;
}

Mat4x4 App::translationMat4(float x, float y, float z) {
  return Mat4x4 {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    x, y, z, 1.0f
  };
}

Mat4x4 App::scaleMat4(float x, float y, float z) {
  return Mat4x4 {
    x, 0.0f, 0.0f, 0.0f,
    0.0f, y, 0.0f, 0.0f,
    0.0f, 0.0f, z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
}