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

  auto it = offscreenObjects.textures.find("colorTexReflection");
  auto it2 = offscreenObjects.textures.find("colorTexRefraction");
  if (it == offscreenObjects.textures.end() || it2 == offscreenObjects.textures.end())
  {
    return;
  }

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, offscreenObjects.textures.at("colorTexReflection"));
  glUniform1i(glGetUniformLocation(program, "reflectionTex"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, offscreenObjects.textures.at("colorTexRefraction"));
  glUniform1i(glGetUniformLocation(program, "refractionTex"), 1);
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

void WaterModule::InitOffscreenFramebuffers()
{
  if (!requiresOffscreenFrameBuffer)
    return;

  {
    unsigned int fbo;
    unsigned int colorTex;
    unsigned int depthTex;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // color texture
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, offscreenObjects.fbWidth, offscreenObjects.fbHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    // depth texture
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, offscreenObjects.fbWidth, offscreenObjects.fbHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cerr << "Framebuffer incomplete!\n";

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    offscreenObjects.framebuffers.emplace("fboReflection", fbo);
    offscreenObjects.textures.emplace("colorTexReflection", colorTex);
    offscreenObjects.textures.emplace("depthTexReflection", depthTex);
  }

  {
    unsigned int fbo;
    unsigned int colorTex;
    unsigned int depthTex;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // color texture
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, offscreenObjects.fbWidth, offscreenObjects.fbHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    // depth texture
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, offscreenObjects.fbWidth, offscreenObjects.fbHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cerr << "Framebuffer incomplete!\n";

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    offscreenObjects.framebuffers.emplace("fboRefraction", fbo);
    offscreenObjects.textures.emplace("colorTexRefraction", colorTex);
    offscreenObjects.textures.emplace("depthTexRefraction", depthTex);
  }
}

void WaterModule::RenderOffscreenFramebuffers(RenderSystem *renderSystem, float deltaTime, const Camera &camera)
{
  glBindFramebuffer(GL_FRAMEBUFFER, offscreenObjects.framebuffers.at("fboReflection"));
  glViewport(0, 0, offscreenObjects.fbWidth, offscreenObjects.fbHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float distance = 2 * (camera.Position.y - waterHeight);
  Camera reflectedCam = camera;
  reflectedCam.Position.y = camera.Position.y - distance;
  reflectedCam.invertPitch();
  glm::mat4 reflectionView = reflectedCam.getViewMatrix();

  renderSystem->RenderScene(deltaTime, reflectedCam, false, true, glm::vec4(0.0f, 1.0f, 0.0f, -waterHeight));

  glBindFramebuffer(GL_FRAMEBUFFER, offscreenObjects.framebuffers.at("fboRefraction"));
  glViewport(0, 0, offscreenObjects.fbWidth, offscreenObjects.fbHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderSystem->RenderScene(deltaTime, camera, false, true, glm::vec4(0.0f, -1.0f, 0.0f, waterHeight));
}