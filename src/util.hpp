#pragma once

#include <math.h>
#include <vector>
#include <SDL3/SDL.h>

namespace App {
  struct Vec2 {
    float x, y;
  };
  struct Vec3 {
    float x, y, z;
  };
  struct Vec4 {
    float x, y, z, w;
  };
  struct Mat4x4 {
    float e00, e01, e02, e03;
    float e10, e11, e12, e13;
    float e20, e21, e22, e23;
    float e30, e31, e32, e33;
    const Uint32 byteSize = 64;
    Mat4x4(
      float i0, float i1, float i2, float i3,
      float i4, float i5, float i6, float i7,
      float i8, float i9, float iA, float iB,
      float iC, float iD, float iE, float iF
    );
    float* rowMajor();
    float* colMajor();
  };
  // Mat4x4 perspectiveMat4(float fovY, float aspectRatio, float near, float far);
  // Mat4x4 orthoMat4(float left, float right, float top, float bottom, float near, float far);
  Mat4x4 translationMat4(float x, float y, float z);
  // Mat4x4 rotationMat4(Vec3 axis, float degree);
  // Mat4x4 rotationEulerMat4(float roll, float pitch, float yaw);
  Mat4x4 scaleMat4(float x, float y, float z);
}