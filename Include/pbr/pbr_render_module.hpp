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
  std::pair<std::string, std::string> GetShaders(RenderSystem *renderSystem, Entity e) const override
  {
    if (!renderSystem->gCoordinator->HasComponent<PBRMaterialComponent>(e))
    {
      return {"", ""};
    }

    return {"", "shaders/pbr_materials/pbr_shader.frag"};
  }

  void UploadUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e) override
  {
    if (!renderSystem->gCoordinator->HasComponent<PBRMaterialComponent>(e))
    {
      return;
    }
    glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, glm::value_ptr(camera.Position));

    glm::vec3 albedo(1.0f, 1.0f, 1.0f);
    std::shared_ptr<Texture> texture = nullptr;

    auto &materialComponent = renderSystem->gCoordinator->GetComponent<PBRMaterialComponent>(e);
    if (materialComponent.material)
    {
      albedo = materialComponent.material->getAlbedo();

      if (materialComponent.material->hasAlbedoMap())
        texture = materialComponent.material->getAlbedoMap();
    }

    glUniform3fv(glGetUniformLocation(program, "materialAlbedo"), 1, glm::value_ptr(albedo));

    // Bind the texture
    if (texture)
    {
      unsigned int slot = 0; // Use slot 0 for base textures
      texture->bind(slot);
      glUniform1i(glGetUniformLocation(program, "albedoMap"), slot);
      glUniform1i(glGetUniformLocation(program, "useAlbedoMap"), true);
    }
    else
    {
      glUniform1i(glGetUniformLocation(program, "useAlbedoMap"), false);
    }

    // Lighting
    // This if statement ignores lighting for all entities with a point light component
    if (!renderSystem->gCoordinator->HasComponent<PointLightComponent>(e))
    {
      glUniform1i(glGetUniformLocation(program, "ignoreLighting"), false);

      int lightCount = 0;
      for (auto const &entityLight : renderSystem->mEntities)
      {
        if (!(renderSystem->gCoordinator->HasComponent<PointLightComponent>(entityLight) && renderSystem->gCoordinator->HasComponent<TransformComponent>(entityLight)))
          continue;

        auto &lightTransform = renderSystem->gCoordinator->GetComponent<TransformComponent>(entityLight);
        auto &lightComponent = renderSystem->gCoordinator->GetComponent<PointLightComponent>(entityLight);

        if (lightCount >= 64)
          break;

        std::string base = "pointLights[" + std::to_string(lightCount) + "]";

        glUniform3fv(glGetUniformLocation(program, (base + ".position").c_str()), 1, glm::value_ptr(lightTransform.translation));
        glUniform3fv(glGetUniformLocation(program, (base + ".color").c_str()), 1, glm::value_ptr(lightComponent.color));
        glUniform1f(glGetUniformLocation(program, (base + ".intensity").c_str()), lightComponent.intensity);
        glUniform1f(glGetUniformLocation(program, (base + ".constant").c_str()), lightComponent.constant);
        glUniform1f(glGetUniformLocation(program, (base + ".linear").c_str()), lightComponent.linear);
        glUniform1f(glGetUniformLocation(program, (base + ".quadratic").c_str()), lightComponent.quadratic);

        lightCount++;
      }

      glUniform1i(glGetUniformLocation(program, "numPointLights"), lightCount);
    }
    else
    {
      glUniform1i(glGetUniformLocation(program, "ignoreLighting"), true);
    }
  }

  void DrawObject(RenderSystem *renderSystem, Entity e) override
  {
    // Does not require a drawable object, so no drawing here
    return;
  }
};