#include <vector>
#include <SDL3/SDL.h>
#include <glm/vec2.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.hpp"

namespace App {
  class ObjectPipeline {
  public:
    ObjectPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu);
    int uploadObject(std::vector<RenderVertex> &vertices);
    int uploadObject(std::vector<RenderVertex> &vertices, std::vector<Uint16> &indices);
    void addTextureToObject(int id, SDL_GPUTextureFormat txFormat, Uint32 w, Uint32 h, bool isRenderTarget);
    void updateObjectPos(int id, glm::vec3 pos);
    void updateObjectRot(int id, glm::vec3 rotAxis, float rotAngle);
    void updateObjectScale(int id, glm::vec3 scale);
    void updateCamera(RenderCamera cam);
    void render(
      SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
      SDL_GPUTexture* target, glm::vec2 targetSize
    );
    void destroy();
  private:
    RenderCamera cam;
    std::vector<RenderObject> robjs;
    SDL_GPUDevice *device = NULL;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUBuffer *debugVertBuffer = NULL;
  };
}