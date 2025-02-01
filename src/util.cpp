#include "util.hpp"

using namespace App;

Mat4x4::Mat4x4(
  float i0, float i1, float i2, float i3,
  float i4, float i5, float i6, float i7,
  float i8, float i9, float iA, float iB,
  float iC, float iD, float iE, float iF
) {
  e00 = i0; e01 = i1; e02 = i2; e03 = i3;
  e10 = i4; e11 = i5; e12 = i6; e13 = i7;
  e20 = i8; e21 = i9; e22 = iA; e23 = iB;
  e30 = iC; e31 = iD; e32 = iE; e33 = iF;
}

float* Mat4x4::rowMajor() {
  static float rm[16] = {
    e00, e01, e02, e03,
    e10, e11, e12, e13,
    e20, e21, e22, e23,
    e30, e31, e32, e33,
  };
  return rm;
}

float* Mat4x4::colMajor() {
  static float cm[16] = {
    e00, e10, e20, e30,
    e01, e11, e21, e31,
    e02, e12, e22, e32,
    e03, e13, e23, e33,
  };
  return cm;
}

Mat4x4 App::translationMat4(float x, float y, float z) {
  return Mat4x4 {
    1.0f, 0.0f, 0.0f, x,
    0.0f, 1.0f, 0.0f, y,
    0.0f, 0.0f, 1.0f, z,
    0.0f, 0.0f, 0.0f, 1.0f
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