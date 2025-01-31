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
    float data[16];
    Uint32 size = 64;
    Mat4x4(
      float i0, float i1, float i2, float i3,
      float i4, float i5, float i6, float i7,
      float i8, float i9, float iA, float iB,
      float iC, float iD, float iE, float iF
    );
    Mat4x4(float values[16]);
    float* array();
  };
  // Mat4x4 perspectiveMat4(float fovY, float aspectRatio, float near, float far);
  // Mat4x4 orthoMat4(float left, float right, float top, float bottom, float near, float far);
  Mat4x4 translationMat4(float x, float y, float z);
  // Mat4x4 rotationMat4(Vec3 axis, float degree);
  // Mat4x4 rotationEulerMat4(float roll, float pitch, float yaw);
  Mat4x4 scaleMat4(float x, float y, float z);
}