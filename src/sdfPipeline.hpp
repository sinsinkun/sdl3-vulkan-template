#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include <glm/vec2.hpp>
#include <glm/ext.hpp>
#include "util.hpp"

namespace App {
  struct SDFRenderObject {
    Uint32 objType = 0;
    float radius = 0.0f;
    glm::vec2 center = glm::vec2(0.0f); // 1st quad
    glm::vec2 v2 = glm::vec2(0.0f);
    glm::vec2 v3 = glm::vec2(0.0f); // 2nd quad
    float cornerRadius = 0.0f;
    float rotation = 0.0f;
    float thickness = 0.0f;
    float padding = 0.0f; // 3rd quad
    SDL_FColor color = WHITE; // 4th quad
  };
  enum SDFObjectType {
    SDF_None, SDF_Circle, SDF_Line, SDF_Triangle, SDF_Rect, SDF_RectA
  };
  class SDFObject {
  public:
    SDFObject() {};
    static SDFObject circle(glm::vec2 center, float radius);
    static SDFObject line(glm::vec2 p1, glm::vec2 p2, float thickness);
    static SDFObject triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
    static SDFObject rect(glm::vec2 center, glm::vec2 size);
    static SDFObject rect(glm::vec2 center, glm::vec2 size, float rotateDeg);
    void withColor(SDL_FColor color);
    void withRoundCorner(float radius);
    void asOutline(float thickness);
    void updatePositionDelta(glm::vec2 delta);
    void updatePosition(glm::vec2 center);
    SDFRenderObject renderObject();
  protected:
    SDFObjectType type = SDF_None;
    glm::vec2 center = glm::vec2(0.0f);
    float radius = 0.0f;
    float cornerRadius = 0.0f;
    float rotation = 0.0f;
    float thickness = 0.0f;
    glm::vec2 v2 = glm::vec2(0.0f);
    glm::vec2 v3 = glm::vec2(0.0f);
    SDL_FColor color = WHITE;
  };
  struct SDFSysData {
    glm::vec2 screenSize;
    glm::vec2 lightPos;
    SDL_FColor lightColor;
    float lightDist;
    Uint32 objCount;
  };
  class SDFPipeline {
  public:
    SDFPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu);
    void refreshObjects(std::vector<SDFObject> objs);
    void render(
      SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
      SDL_GPUTexture* target, SDFSysData sys
    );
    void destroy();
  private:
    SDL_GPUDevice *device;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUBuffer *objsBuffer = NULL;
  };
  // sdf math
  float sdfToCir(glm::vec2 point, glm::vec2 center, float radius);
  float sdfToLine(glm::vec2 point, glm::vec2 p1, glm::vec2 p2);
  float sdfToTriangle(glm::vec2 point, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
  float sdfToRect(glm::vec2 point, glm::vec2 center, glm::vec2 size);
  float sdfWithCorner(float sdf, float radius);
  float sdfAsOutline(float sdf, float thickness);
  float calculateSdf(glm::vec2 point, float maxDist, std::vector<SDFObject> *objs);
  float calculateRayMarch(glm::vec2 point, glm::vec2 direction, float maxDist, std::vector<SDFObject> *objs);
}