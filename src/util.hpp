#pragma once

#include <vector>
#include <SDL3/SDL.h>

namespace App {
  // GPU helpers
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
}