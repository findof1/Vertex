#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "core/camera.hpp"
#include "core/ecs/coordinator.hpp"
#include "core/ecs/components.hpp"
#include "core/model.hpp"
#include "core/material.hpp"
#include "core/ecs/render_system.hpp"
#include <core/texture_manager.hpp>
#include <core/ecs/core_render_module.hpp>
#include "pbr/pbr_material.hpp"
#include "pbr/components.hpp"
#include "pbr/pbr_render_module.hpp"
#include <animations/components.hpp>
#include <animations/animations_render_module.hpp>
#include "animations/animated_model.hpp"

Camera camera;

float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
  if (firstMouse)
  {
    lastX = (float)xpos;
    lastY = (float)ypos;
    firstMouse = false;
  }

  float xoffset = (float)xpos - lastX;
  float yoffset = lastY - (float)ypos;

  lastX = (float)xpos;
  lastY = (float)ypos;

  camera.processMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow *window, float deltaTime)
{
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.processKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.processKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.processKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.processKeyboard(RIGHT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    camera.processKeyboard(UP, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    camera.processKeyboard(DOWN, deltaTime);
}

const int WIDTH = 1600;
const int HEIGHT = 1200;

int main()
{
  if (!glfwInit())
    return -1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vertex Core Module Demo", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);

  // Initialize ECS
  auto coordinator = std::make_shared<Coordinator>();
  coordinator->Init();

  // Register components
  coordinator->RegisterComponent<TransformComponent>();
  coordinator->RegisterComponent<ModelComponent>();
  coordinator->RegisterComponent<MaterialComponent>();
  coordinator->RegisterComponent<PointLightComponent>();
  coordinator->RegisterComponent<PBRMaterialComponent>();
  coordinator->RegisterComponent<AnimatedModelComponent>();

  // Register and configure render system
  auto renderSystem = coordinator->RegisterSystem<RenderSystem>();
  {
    Signature signature;
    signature.set(coordinator->GetComponentType<TransformComponent>());
    coordinator->SetSystemSignature<RenderSystem>(signature);
  }
  renderSystem->Init(coordinator);
  renderSystem->AddModule(std::make_unique<CoreObjectModule>());
  renderSystem->AddModule(std::make_unique<CoreLightingModule>());
  renderSystem->AddModule(std::make_unique<PBRLightingModule>());
  renderSystem->AddModule(std::make_unique<AnimationsObjectModule>());

  // Initiate Texture Manager
  TextureManager textureManager;

  // load assets
  auto fireTexture = textureManager.load("assets/textures/fire.png");
  auto manTexture = textureManager.load("assets/models/animationMan/Ch31_1001_Diffuse.png");
  auto manModel = AnimatedModel::createModelFromFile("assets/models/animationMan/man.gltf");
  auto cubeModel = Model::createModelFromFile("assets/models/cube.obj");
  auto vaseModel = Model::createModelFromFile("assets/models/smooth_vase.obj");

  // Create a cube entity
  {
    Entity cube = coordinator->CreateEntity();
    TransformComponent cubeTransform{};
    cubeTransform.translation = {0.0f, 0.0f, -5.0f};
    cubeTransform.scale = {1.0f, 1.0f, 1.0f};
    coordinator->AddComponent(cube, cubeTransform);
    coordinator->AddComponent(cube, ModelComponent{cubeModel});
    PBRMaterialComponent cubeMat{std::make_shared<PBRMaterial>()};
    cubeMat.material->setAlbedo(glm::vec3(0.0f, 1.0f, 0.3f));
    cubeMat.material->setAlbedoMap(fireTexture);
    coordinator->AddComponent(cube, cubeMat);
  }

  // Create a man entity
  {
    Entity man = coordinator->CreateEntity();
    TransformComponent manTransform{};
    manTransform.translation = {5.0f, 0.0f, 0.0f};
    manTransform.scale = {1.0f, 1.0f, 1.0f};
    coordinator->AddComponent(man, manTransform);
    coordinator->AddComponent(man, AnimatedModelComponent{manModel});
    PBRMaterialComponent manMat{std::make_shared<PBRMaterial>()};
    manMat.material->setAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
    manMat.material->setAlbedoMap(manTexture);
    coordinator->AddComponent(man, manMat);
  }

  // Create a vase entity
  {
    Entity vase = coordinator->CreateEntity();
    TransformComponent vaseTransform{};
    vaseTransform.translation = {0.0f, -1.0f, 5.0f};
    vaseTransform.scale = {7.0f, 7.0f, 7.0f};
    vaseTransform.rotation = {180.0f, 0.0f, 0.0f};
    coordinator->AddComponent(vase, vaseTransform);
    coordinator->AddComponent(vase, ModelComponent{vaseModel});
    MaterialComponent vaseMat{std::make_shared<Material>()};
    vaseMat.material->setAlbedo(glm::vec3(0.9f, 0.1f, 0.1f));
    coordinator->AddComponent(vase, vaseMat);
  }

  // Create a point light
  {
    Entity light = coordinator->CreateEntity();
    TransformComponent lightTransform{};
    lightTransform.translation = {0.0f, 2.0f, 0.0f};
    lightTransform.scale = {0.25f, 0.25f, 0.25f};
    lightTransform.rotation = {0.0f, 0.0f, 0.0f};
    coordinator->AddComponent(light, lightTransform);
    coordinator->AddComponent(light, ModelComponent{cubeModel});
    coordinator->AddComponent(light, PointLightComponent{});
    PBRMaterialComponent lightMat{std::make_shared<PBRMaterial>()};
    lightMat.material->setAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
    coordinator->AddComponent(light, lightMat);
  }

  float lastTime = glfwGetTime();
  while (!glfwWindowShouldClose(window))
  {
    float time = glfwGetTime();
    float dt = time - lastTime;
    lastTime = time;

    processInput(window, dt);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update and render using ECS
    renderSystem->Update(dt, camera);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}