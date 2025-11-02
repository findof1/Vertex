#include "pbr/pbr_render_module.hpp"

std::pair<std::string, std::string> PBRLightingModule::GetShaders(RenderSystem *renderSystem, Entity e) const
{
  if (!renderSystem->gCoordinator->HasComponent<PBRMaterialComponent>(e))
  {
    return {"", ""};
  }

  return {"", "shaders/pbr_materials/pbr_shader.frag"};
}

void PBRLightingModule::UploadObjectUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e)
{
  if (!renderSystem->gCoordinator->HasComponent<PBRMaterialComponent>(e))
  {
    return;
  }
  glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, glm::value_ptr(camera.Position));

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

void PBRLightingModule::UploadMeshUniforms(unsigned int program, RenderSystem *renderSystem, Entity e, int materialID)
{
  if (!renderSystem->gCoordinator->HasComponent<PBRMaterialComponent>(e))
  {
    return;
  }

  glm::vec3 albedo(1.0f, 1.0f, 1.0f);
  std::shared_ptr<Texture> texture = nullptr;

  auto &materialComponent = renderSystem->gCoordinator->GetComponent<PBRMaterialComponent>(e);

  std::shared_ptr<PBRMaterial> material = nullptr;
  if (materialID == -1 && !materialComponent.materials.empty())
  {
    material = materialComponent.materials.at(0);
  }
  else
  {
    for (const auto &mat : materialComponent.materials)
    {
      if (mat->id == materialID)
      {
        material = mat;
        break;
      }
    }
  }

  if (material)
  {
    albedo = material->getAlbedo();

    if (material->hasAlbedoMap())
      texture = material->getAlbedoMap();
  }

  glUniform3fv(glGetUniformLocation(program, "materialAlbedo"), 1, glm::value_ptr(albedo));
  if (material->ignoreLighting)
  {
    glUniform1i(glGetUniformLocation(program, "ignoreLighting"), GL_TRUE);
  }

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
}