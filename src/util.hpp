#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace App {
  // GPU helpers
  enum GPUPrimitiveType { PT_Point, PT_Line, PT_Tri };
  struct RenderVertex {
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
  };
  struct RenderObject {
    int id = -1;
    bool visible = true;
    SDL_GPUBuffer *vertexBuffer = NULL;
    SDL_GPUBuffer *indexBuffer = NULL;
    int vertexCount = 0;
    int indexCount = 0;
    SDL_GPUSampler *sampler = NULL;
    SDL_GPUTexture *texture = NULL;
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 rotAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    float rotAngleRad = 0.0f;
    SDL_FColor albedo {0.5f, 0.5f, 0.5f, 1.0f};
  };
  struct RenderCamera {
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, 1.0f);
    bool perspective = false;
    float near = 0.01f;
    float far = 1000.0f;
    float viewWidth = 0.0f;
    float viewHeight = 0.0f;
    float fovY = 1.05f;
  };
  SDL_GPUShader* loadShader(
    SDL_GPUDevice *device, const char* filename, Uint32 samplerCount,
    Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount
  );
  void copyVertexDataIntoBuffer(
    SDL_GPUDevice *device, SDL_GPUBuffer *vertBuf, SDL_GPUBuffer *indexBuf,
    std::vector<RenderVertex> *verts, std::vector<Uint16> *indices
  );
  SDL_GPUVertexInputState createVertexInputState();
  // color
  SDL_FColor rgba(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  SDL_FColor rgb(Uint8 r, Uint8 g, Uint8 b);
  SDL_FColor hsva(float h, float s, float v, float a);
  SDL_FColor hsv(float h, float s, float v);
  SDL_FColor modAlpha(SDL_FColor in, float a);
  static SDL_FColor TRANSPARENT{0.0f, 0.0f, 0.0f, 0.0f};
  static SDL_FColor BLACK{0.0f, 0.0f, 0.0f, 1.0f};
  static SDL_FColor GRAY{0.5f, 0.5f, 0.5f, 1.0f};
  static SDL_FColor WHITE{1.0f, 1.0f, 1.0f, 1.0f};
  static SDL_FColor RED{1.0f, 0.0f, 0.0f, 1.0f};
  static SDL_FColor GREEN{0.0f, 1.0f, 0.0f, 1.0f};
  static SDL_FColor BLUE{0.0f, 0.0f, 1.0f, 1.0f};
  static SDL_FColor YELLOW{1.0f, 1.0f, 0.0f, 1.0f};
  static SDL_FColor CYAN{0.0f, 1.0f, 1.0f, 1.0f};
  static SDL_FColor MAGENTA{1.0f, 0.0f, 1.0f, 1.0f};
  static SDL_FColor ORANGE{1.0f, 0.5f, 0.0f, 1.0f};
  static SDL_FColor PURPLE{0.5f, 0.0f, 1.0f, 1.0f};
  // math
  float degToRad(float d);
  float radToDeg(float r);
  // primitives
  struct Primitive {
    std::vector<RenderVertex> vertices;
    std::vector<Uint16> indices;
    bool useIndices = true;
  };
  Primitive rect2d(float w, float h, float z);
  Primitive regPolygon2d(float radius, Uint16 sides, float z);
  Primitive torus2d(float outerRadius, float innerRadius, Uint16 sides, float z);
  Primitive cube(float w, float h, float d);
  Primitive cylinder(float r, float h, Uint16 sides);
  Primitive tube(float outerRadius, float innerRadius, float h, Uint16 sides);
  Primitive sphere(float r, Uint16 sides, Uint16 slices);
  Primitive hemisphere(float r, Uint16 sides, Uint16 slices);
}