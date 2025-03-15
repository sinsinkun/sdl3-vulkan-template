#include <vector>
#include <SDL3/SDL.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.hpp"

namespace App {
  struct LightMaterial {
    SDL_FColor lightColor = BLACK;
    glm::vec3 lightPos = glm::vec3(0.0f);
    float lightMaxDist = 1000.0f;
    float ambientIntensity = 0.0f;
    float specularIntensity = 0.0f;
    float shininess = 32.0f;
    float padding = 0.0f;
  };
  struct PhongMaterial : LightMaterial {
    SDL_FColor albedo = GRAY;
    glm::vec3 cameraPos = glm::vec3(0.0f);
    PhongMaterial(LightMaterial const &parent) : LightMaterial(parent) {};
  };
  class ObjectPipeline {
  public:
    ObjectPipeline(
      SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu,
      GPUPrimitiveType type, SDL_GPUCullMode cullMode, Uint32 sw, Uint32 sh
    );
    void resizeScreen(Uint32 w, Uint32 h);
    int uploadObject(std::vector<RenderVertex> const &vertices);
    int uploadObject(std::vector<RenderVertex> const &vertices, std::vector<Uint16> const &indices);
    int uploadObject(Primitive const &shape);
    void addTextureToObject(int id, SDL_GPUTexture *texture);
    RenderObject& getObject(int id);
    void render(SDL_GPUCommandBuffer *cmdBuf, SDL_GPUTexture* target, LightMaterial const &light);
    void clearObjects();
    void destroy();
    RenderCamera cam;
  private:
    std::vector<RenderObject> robjs;
    SDL_GPUDevice *device = NULL;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUTexture *depthTx = NULL;
  };
}