#pragma once
#include "core/ecs/render_system.hpp"
#include <glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "components.hpp"
#include "core/texture.hpp"

// Used for sending data to the gpu for water meshes
class WaterModule : public RenderModule
{
public:
  WaterModule(float waterHeight = 0) : waterHeight(waterHeight)
  {
    requiresOffscreenFrameBuffer = true;
  }

  float waterHeight;

  void RenderOffscreenFramebuffers(RenderSystem *renderSystem, float deltaTime, const Camera &camera) override;

  void InitOffscreenFramebuffers() override;

  std::pair<std::string, std::string> GetShaders(RenderSystem *renderSystem, Entity e) const override;

  void UploadObjectUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e) override;

  void UploadMeshUniforms(unsigned int program, RenderSystem *renderSystem, Entity e, int materialID) override;

  void DrawObject(unsigned int program, RenderSystem *renderSystem, Entity e) override;
};