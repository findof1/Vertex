#include "animations/animations_render_module.hpp"
#include "animations/animated_model.hpp"
#include "animations/components.hpp"

std::pair<std::string, std::string> AnimationsObjectModule::GetShaders(RenderSystem *renderSystem, Entity e) const
{
  if (!renderSystem->gCoordinator->HasComponent<AnimatedModelComponent>(e))
  {
    return {"", ""};
  }
  return {"shaders/animated_meshes/animated_shader.vert", ""};
}

void AnimationsObjectModule::UploadObjectUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e)
{
  if (!renderSystem->gCoordinator->HasComponent<AnimatedModelComponent>(e))
  {
    return;
  }

  glm::mat4 view = camera.getViewMatrix();
  glm::mat4 proj = camera.getProjectionMatrix(16.0f / 12.0f);

  glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

  auto &animModelComp = renderSystem->gCoordinator->GetComponent<AnimatedModelComponent>(e);
  auto &animatedModel = *animModelComp.model;

  const std::vector<glm::mat4> &boneMatrices = animatedModel.GetFinalBoneMatrices();

  for (size_t i = 0; i < boneMatrices.size() && i < 100; ++i)
  {
    std::string name = "boneMatrices[" + std::to_string(i) + "]";
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(boneMatrices[i]));
  }

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

void AnimationsObjectModule::DrawObject(unsigned int program, RenderSystem *renderSystem, Entity e)
{
  if (!renderSystem->gCoordinator->HasComponent<AnimatedModelComponent>(e))
  {
    return;
  }
  for (const auto &mesh : renderSystem->gCoordinator->GetComponent<AnimatedModelComponent>(e).model->meshes)
  {
    for (auto &module : renderSystem->modules)
    {
      module->UploadMeshUniforms(program, renderSystem, e, mesh->textureID);
    }
    mesh->Draw();
  }
}