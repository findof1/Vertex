#include "core/ecs/core_render_module.hpp"
#include "core/model.hpp"

std::pair<std::string, std::string> CoreLightingModule::GetShaders(RenderSystem *renderSystem, Entity e) const
{
  if (!renderSystem->gCoordinator->HasComponent<MaterialComponent>(e))
  {
    return {"", ""};
  }

  return {"", "shaders/basic_materials/default_shader.frag"};
}

void CoreLightingModule::UploadUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e)
{
  if (!renderSystem->gCoordinator->HasComponent<MaterialComponent>(e))
  {
    return;
  }
  glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, glm::value_ptr(camera.Position));

  glm::vec3 albedo(1.0f, 1.0f, 1.0f);
  std::shared_ptr<Texture> texture = nullptr;

  auto &materialComponent = renderSystem->gCoordinator->GetComponent<MaterialComponent>(e);
  if (materialComponent.material)
  {
    albedo = materialComponent.material->getAlbedo();

    if (materialComponent.material->hasTexture())
      texture = materialComponent.material->getTexture();
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

std::pair<std::string, std::string> CoreObjectModule::GetShaders(RenderSystem *renderSystem, Entity e) const
{
  if (!renderSystem->gCoordinator->HasComponent<ModelComponent>(e))
  {
    return {"", ""};
  }
  return {"shaders/basic_materials/default_shader.vert", ""};
}

void CoreObjectModule::UploadUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e)
{
  if (!renderSystem->gCoordinator->HasComponent<ModelComponent>(e))
  {
    return;
  }

  glm::mat4 view = camera.getViewMatrix();
  glm::mat4 proj = camera.getProjectionMatrix(16.0f / 12.0f);

  glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

  if (!renderSystem->gCoordinator->HasComponent<TransformComponent>(e))
  {
    glm::mat4 defaultModel(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(defaultModel));
    return;
  }

  auto &transform = renderSystem->gCoordinator->GetComponent<TransformComponent>(e);
  glm::mat4 model = transform.GetMatrix();
  glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
}

void CoreObjectModule::DrawObject(RenderSystem *renderSystem, Entity e)
{
  if (!renderSystem->gCoordinator->HasComponent<ModelComponent>(e))
  {
    return;
  }
  renderSystem->gCoordinator->GetComponent<ModelComponent>(e).model->Draw();
}