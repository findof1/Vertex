#pragma once
#include "render_system.hpp"
#include <glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "components.hpp"
#include "../material.hpp"
#include "../texture.hpp"

// Used for sending data to the fragment shader for all basic phong materials
// Note: Does not draw objects because it does not require a drawable component such as model
class CoreLightingModule : public RenderModule
{
public:
  std::pair<std::string, std::string> GetShaders(RenderSystem *renderSystem, Entity e) const;

  void UploadUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e) override;

  void DrawObject(RenderSystem *renderSystem, Entity e) override
  {
    // Does not require a drawable object, so no drawing here
    // In other words, since this module only requires material, and material is not a drawable component, let the module that requires the model component to draw instead
    return;
  }
};

// Used for sending data to the vertex shader for all basic models and drawing objects with a model component
class CoreObjectModule : public RenderModule
{
public:
  std::pair<std::string, std::string> GetShaders(RenderSystem *renderSystem, Entity e) const override;

  void UploadUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e) override;

  void DrawObject(RenderSystem *renderSystem, Entity e) override;
};