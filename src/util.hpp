#pragma once

#include <vector>
#include <SDL3/SDL.h>

namespace App {
  enum GPUPrimitiveType { Point, Line, Triangle };
  struct RenderVertex {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
    float r, g, b, a;
  };
  struct RenderObject {
    SDL_GPUBuffer *vertexBuffer = NULL;
    SDL_GPUBuffer *indexBuffer = NULL;
    int vertexCount = 0;
    int indexCount = 0;
  };
  SDL_GPUShader* loadShader(
    SDL_GPUDevice *device, const char* filename, Uint32 samplerCount,
    Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount
  );
  SDL_GPUVertexInputState createVertexInputState();
  struct Vec2 {
    float x, y;
    Vec2(float a) { x = a; y = a; }
    Vec2(float x, float y) {
      this->x = x; this->y = y;
    }
    Vec2 operator+(Vec2 b) {
      return Vec2{ x + b.x, y + b.y };
    }
    Vec2 operator-(Vec2 b) {
      return Vec2{ x - b.x, y - b.y };
    }
    Vec2 operator*(float b) {
      return Vec2{ x * b, y * b};
    }
    Vec2 operator/(float b) {
      return Vec2{ x / b, y / b};
    }
  };
  struct Vec3 {
    float x, y, z;
    Vec3(float a) { x = a; y = a; z = a; }
    Vec3(float x, float y, float z) {
      this->x = x; this->y = y; this->z = z;
    }
    Vec3 operator+(Vec3 b) {
      return Vec3{ x + b.x, y + b.y, z + b.z };
    }
    Vec3 operator-(Vec3 b) {
      return Vec3{ x - b.x, y - b.y, z - b.z };
    }
    Vec3 operator*(float b) {
      return Vec3{ x * b, y * b, z * b };
    }
    Vec3 operator/(float b) {
      return Vec3{ x / b, y / b, z / b };
    }
  };
  struct Vec4 {
    float x, y, z, w;
    Vec4(float a) { x = a; y = a; z = a; w = a; }
    Vec4(float x, float y, float z, float w ) {
      this->x = x; this->y = y; this->z = z; this->w = w;
    }
    Vec4 operator+(Vec4 b) {
      return Vec4{ x + b.x, y + b.y, z + b.z, w + b.w };
    }
    Vec4 operator-(Vec4 b) {
      return Vec4{ x - b.x, y - b.y, z - b.z, w - b.w };
    }
    Vec4 operator*(float b) {
      return Vec4{ x * b, y * b, z * b, w * b };
    }
    Vec4 operator/(float b) {
      return Vec4{ x / b, y / b, z / b, w / b };
    }
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
  Mat4x4 identityMat4();
  // Mat4x4 perspectiveMat4(float fovY, float aspectRatio, float near, float far);
  // Mat4x4 orthoMat4(float left, float right, float top, float bottom, float near, float far);
  Mat4x4 translationMat4(float x, float y, float z);
  // Mat4x4 rotationQuatMat4(Vec3 axis, float degree);
  // Mat4x4 rotationEulerMat4(float roll, float pitch, float yaw);
  Mat4x4 scaleMat4(float x, float y, float z);
}