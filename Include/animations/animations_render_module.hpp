#pragma once
#include "core/ecs/render_system.hpp"
#include <glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "components.hpp"
#include "core/texture.hpp"

// Used for sending data to the vertex shader for all animated models and drawing objects with a model component
class AnimationsObjectModule : public RenderModule
{
public:
  std::pair<std::string, std::string> GetShaders(RenderSystem *renderSystem, Entity e) const override;

  void UploadUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e) override;

  void DrawObject(RenderSystem *renderSystem, Entity e) override;
};