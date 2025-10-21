#pragma once
#include "core/ecs/render_system.hpp"
#include <glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "components.hpp"
#include "pbr_material.hpp"
#include "core/texture.hpp"

// Used for sending data to the fragment shader for all pbr materials
class PBRLightingModule : public RenderModule
{
public:
  std::pair<std::string, std::string> GetShaders(RenderSystem *renderSystem, Entity e) const override;

  void UploadUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e) override;

  void DrawObject(RenderSystem *renderSystem, Entity e) override
  {
    // Does not require a drawable object, so no drawing here
    return;
  }
};