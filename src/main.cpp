#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
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
#include "animations/animations_system.hpp"
#include "animations/animation.hpp"
#include "physics/physics_system.hpp"
#include "physics/rigidbody.hpp"
#include "water/components.hpp"
#include "water/water_mesh.hpp"
#include <water/water_render_module.hpp>

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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
  coordinator->RegisterComponent<AnimationComponent>();
  coordinator->RegisterComponent<RigidbodyComponent>();
  coordinator->RegisterComponent<ColliderComponent>();
  coordinator->RegisterComponent<WaterMeshComponent>();

  // Register and configure animations system
  auto animationsSystem = coordinator->RegisterSystem<AnimationsSystem>();
  {
    Signature signature;
    signature.set(coordinator->GetComponentType<AnimationComponent>());
    signature.set(coordinator->GetComponentType<AnimatedModelComponent>());
    coordinator->SetSystemSignature<AnimationsSystem>(signature);
  }
  animationsSystem->Init(coordinator);

  // Register and configure render system
  auto renderSystem = coordinator->RegisterSystem<RenderSystem>();
  {
    Signature signature;
    signature.set(coordinator->GetComponentType<TransformComponent>());
    coordinator->SetSystemSignature<RenderSystem>(signature);
  }
  renderSystem->Init(coordinator, WIDTH, HEIGHT);
  renderSystem->AddModule(std::make_unique<CoreObjectModule>());
  renderSystem->AddModule(std::make_unique<CoreLightingModule>());
  renderSystem->AddModule(std::make_unique<PBRLightingModule>());
  renderSystem->AddModule(std::make_unique<AnimationsObjectModule>());
  renderSystem->AddModule(std::make_unique<WaterModule>(-7));

  // Register and configure physics system
  auto physicsSystem = coordinator->RegisterSystem<PhysicsSystem>();
  {
    Signature signature;
    signature.set(coordinator->GetComponentType<RigidbodyComponent>());
    signature.set(coordinator->GetComponentType<TransformComponent>());
    coordinator->SetSystemSignature<PhysicsSystem>(signature);
  }

  // Initiate Texture Manager
  TextureManager textureManager;

  // load assets
  auto fireTexture = textureManager.load("assets/textures/fire.png");
  auto skyTexture = textureManager.load("assets/textures/sky3.png");
  auto manTextures = Material::createModelMaterialsFromFile(&textureManager, "assets/models/animationMan/man.gltf");
  auto manTexturesPBR = PBRMaterial::createModelMaterialsFromFile(&textureManager, "assets/models/animationMan/man.gltf");
  auto manModel = AnimatedModel::createModelFromFile("assets/models/animationMan/man.gltf");
  auto manModel2 = AnimatedModel::createModelFromFile("assets/models/animationMan/man.gltf");
  std::shared_ptr<Animation> manAnimation = std::make_shared<Animation>("assets/models/animationMan/man.gltf", manModel->boneMapping);
  auto cubeModel = Model::createModelFromFile("assets/models/cube.obj", false);
  auto skyboxModel = Model::createModelFromFile("assets/models/skybox.obj", false);
  auto vaseModel = Model::createModelFromFile("assets/models/smooth_vase.obj", false);

  auto waterDUDV = textureManager.load("assets/textures/waterdudv.png");
  auto waterNormals = textureManager.load("assets/textures/waternormal.png");
  std::shared_ptr<WaterMesh> waterMesh = std::make_shared<WaterMesh>(200, 100, 24, waterDUDV, waterNormals);

  // Create a cube entity
  {
    Entity cube = coordinator->CreateEntity();
    TransformComponent cubeTransform{};
    cubeTransform.translation = {0.0f, 0.0f, -5.0f};
    cubeTransform.scale = {1.0f, 1.0f, 1.0f};
    coordinator->AddComponent(cube, cubeTransform);
    coordinator->AddComponent(cube, ModelComponent{cubeModel});
    PBRMaterialComponent cubeMat{std::make_shared<PBRMaterial>()};
    cubeMat.materials.at(0)->setAlbedo(glm::vec3(0.0f, 1.0f, 0.3f));
    cubeMat.materials.at(0)->setAlbedoMap(fireTexture);
    coordinator->AddComponent(cube, cubeMat);
    auto cubeRB = std::make_shared<RigidBody>();
    cubeRB->mass = 2.0f;
    cubeRB->useGravity = true;
    cubeRB->gravity = glm::vec3(0.0f, -1.84f, 0.0f);
    RigidbodyComponent cubeRigidbody{cubeRB};
    coordinator->AddComponent(cube, cubeRigidbody);
  }

  // Create a ground entity
  {
    Entity ground = coordinator->CreateEntity();
    TransformComponent groundTransform{};
    groundTransform.translation = {0.0f, -15.0f, 0.0f};
    groundTransform.scale = {80.0f, 1.0f, 80.0f};
    coordinator->AddComponent(ground, groundTransform);
    coordinator->AddComponent(ground, ModelComponent{cubeModel});
    PBRMaterialComponent groundMat{std::make_shared<PBRMaterial>()};
    groundMat.materials.at(0)->setAlbedo(glm::vec3(0.7f, 0.45f, 0.05f));
    coordinator->AddComponent(ground, groundMat);
    auto groundRB = std::make_shared<RigidBody>();
    groundRB->mass = 1000.0f;
    groundRB->useGravity = false;
    groundRB->gravity = glm::vec3(0.0f, 0.0f, 0.0f);
    RigidbodyComponent groundRigidbody{groundRB};
    coordinator->AddComponent(ground, groundRigidbody);
  }

  // Create a skybox entity
  {
    Entity cube = coordinator->CreateEntity();
    TransformComponent cubeTransform{};
    cubeTransform.translation = {0.0f, 0.0f, 0.0f};
    cubeTransform.scale = {100.0f, 100.0f, 100.0f};
    coordinator->AddComponent(cube, cubeTransform);
    coordinator->AddComponent(cube, ModelComponent{skyboxModel});
    PBRMaterialComponent cubeMat{std::make_shared<PBRMaterial>()};
    cubeMat.materials.at(0)->setAlbedoMap(skyTexture);
    cubeMat.materials.at(0)->ignoreLighting = true;
    coordinator->AddComponent(cube, cubeMat);
  }

  // Create a water entity
  {
    Entity water = coordinator->CreateEntity();
    TransformComponent waterTransform{};
    waterTransform.translation = {0.0f, -7.0f, 0.0f};
    waterTransform.scale = {1.0f, 1.0f, 1.0f};
    coordinator->AddComponent(water, waterTransform);
    coordinator->AddComponent(water, WaterMeshComponent{waterMesh});
  }

  // Create a phong man entity
  {
    Entity man = coordinator->CreateEntity();
    TransformComponent manTransform{};
    manTransform.translation = {3.0f, 0.0f, 0.0f};
    manTransform.scale = {1.0f, 1.0f, 1.0f};
    manTransform.rotation = {0.0f, -90.0f, 0.0f};
    coordinator->AddComponent(man, manTransform);
    coordinator->AddComponent(man, AnimatedModelComponent{manModel});
    MaterialComponent manMat;
    manMat.materials = manTextures;
    coordinator->AddComponent(man, manMat);
  }

  // Create a pbr man entity
  {
    Entity man = coordinator->CreateEntity();
    TransformComponent manTransform{};
    manTransform.translation = {-3.0f, 0.0f, 0.0f};
    manTransform.scale = {1.0f, 1.0f, 1.0f};
    manTransform.rotation = {0.0f, 90.0f, 0.0f};
    coordinator->AddComponent(man, manTransform);
    coordinator->AddComponent(man, AnimatedModelComponent{manModel2});
    PBRMaterialComponent manMat;
    manMat.materials = manTexturesPBR;
    coordinator->AddComponent(man, manMat);
    AnimationComponent anim;
    anim.animation = manAnimation;
    coordinator->AddComponent(man, anim);
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
    vaseMat.materials.at(0)->setAlbedo(glm::vec3(0.9f, 0.1f, 0.1f));
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
    lightMat.materials.at(0)->setAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
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

    // Update physics before rendering
    physicsSystem->Update(coordinator, dt);

    // Update animations before rendering
    animationsSystem->Update(dt, camera);

    // Render
    renderSystem->Update(dt, camera);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}