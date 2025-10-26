#include "water/water_render_module.hpp"
#include "water/water_mesh.hpp"
#include <GLFW/glfw3.h>

std::pair<std::string, std::string> WaterModule::GetShaders(RenderSystem *renderSystem, Entity e) const
{
  if (!renderSystem->gCoordinator->HasComponent<WaterMeshComponent>(e))
  {
    return {"", ""};
  }

  return {"shaders/water_shader/water.vert", "shaders/water_shader/water.frag"};
}

void WaterModule::UploadObjectUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity e)
{
  if (!renderSystem->gCoordinator->HasComponent<WaterMeshComponent>(e))
  {
    return;
  }
  glm::mat4 view = camera.getViewMatrix();
  glm::mat4 proj = camera.getProjectionMatrix(16.0f / 12.0f);

  glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

  const auto water = renderSystem->gCoordinator->GetComponent<WaterMeshComponent>(e);
  const WaterMaterial &material = water.water->material;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, water.water->envMap);
  glUniform1i(glGetUniformLocation(program, "envMap"), 0);

  glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1, glm::value_ptr(camera.Position));
  glUniform3fv(glGetUniformLocation(program, "sunDir"), 1, glm::value_ptr(material.sunDir));
  glUniform3fv(glGetUniformLocation(program, "sunColor"), 1, glm::value_ptr(material.sunColor));
  glUniform3fv(glGetUniformLocation(program, "deepColor"), 1, glm::value_ptr(material.deepColor));
  glUniform1f(glGetUniformLocation(program, "foamThreshold"), material.foamThreshold);

  const std::vector<WaterWave> &waves = water.water->waves;
  int numWaves = static_cast<int>(waves.size());
  if (numWaves > 32) // 32 is the shader limit
    numWaves = 32;

  glUniform1i(glGetUniformLocation(program, "numWaves"), numWaves);

  for (int i = 0; i < numWaves; i++)
  {
    const WaterWave &w = waves[i];

    std::string prefix = "waves[" + std::to_string(i) + "].";

    glUniform2fv(glGetUniformLocation(program, (prefix + "dir").c_str()), 1, glm::value_ptr(w.direction));
    glUniform1f(glGetUniformLocation(program, (prefix + "wavelength").c_str()), w.wavelength);
    glUniform1f(glGetUniformLocation(program, (prefix + "amplitude").c_str()), w.amplitude);
    glUniform1f(glGetUniformLocation(program, (prefix + "speed").c_str()), w.speed);
    glUniform1f(glGetUniformLocation(program, (prefix + "steepness").c_str()), w.steepness);
  }

  float time = static_cast<float>(glfwGetTime());
  glUniform1f(glGetUniformLocation(program, "time"), time);

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

void WaterModule::UploadMeshUniforms(unsigned int program, RenderSystem *renderSystem, Entity e, int materialID)
{
}

void WaterModule::DrawObject(unsigned int program, RenderSystem *renderSystem, Entity e)
{
  if (!renderSystem->gCoordinator->HasComponent<WaterMeshComponent>(e))
  {
    return;
  }

  renderSystem->gCoordinator->GetComponent<WaterMeshComponent>(e).water->Draw();
}