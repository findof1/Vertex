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
#include "core/components.hpp"
#include "core/model.hpp"
#include "core/material.hpp"
#include "core/render_system.hpp"

std::string loadShaderSource(const char *filepath)
{
  std::ifstream file(filepath);
  if (!file.is_open())
  {
    std::cerr << "Failed to open shader file: " << filepath << std::endl;
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

unsigned int compileShader(const char *source, GLenum type)
{
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << "Shader compile error:\n"
              << infoLog << std::endl;
  }
  return shader;
}

unsigned int createShaderProgram(const char *vertexPath, const char *fragmentPath)
{
  std::string vSrc = loadShaderSource(vertexPath);
  std::string fSrc = loadShaderSource(fragmentPath);

  unsigned int vShader = compileShader(vSrc.c_str(), GL_VERTEX_SHADER);
  unsigned int fShader = compileShader(fSrc.c_str(), GL_FRAGMENT_SHADER);

  unsigned int program = glCreateProgram();
  glAttachShader(program, vShader);
  glAttachShader(program, fShader);
  glLinkProgram(program);

  int success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cerr << "Program link error:\n"
              << infoLog << std::endl;
  }

  glDeleteShader(vShader);
  glDeleteShader(fShader);

  return program;
}

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
}

const int WIDTH = 1600;
const int HEIGHT = 1200;

int main()
{
  if (!glfwInit()) return -1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vertex ECS Demo", NULL, NULL);
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

  // Register and configure render system
  auto renderSystem = coordinator->RegisterSystem<RenderSystem>();
  {
    Signature signature;
    signature.set(coordinator->GetComponentType<TransformComponent>());
    signature.set(coordinator->GetComponentType<ModelComponent>());
    coordinator->SetSystemSignature<RenderSystem>(signature);
  }
  renderSystem->Init(coordinator);

  // Create shader program
  unsigned int shaderProgram = createShaderProgram(
      "shaders/shader.vert",
      "shaders/shader.frag");

  // Create a cube entity
  auto cubeModel = Model::createModelFromFile("res/models/cube.obj");
  Entity cube = coordinator->CreateEntity();
  TransformComponent cubeTransform{};
  cubeTransform.translation = {0.0f, 0.0f, -5.0f};
  cubeTransform.scale = {1.0f, 1.0f, 1.0f};
  coordinator->AddComponent(cube, cubeTransform);
  coordinator->AddComponent(cube, ModelComponent{cubeModel});
  MaterialComponent cubeMat{std::make_shared<Material>()};
  cubeMat.material->setColor(glm::vec3(0.0f, 1.0f, 0.3f));
  coordinator->AddComponent(cube, cubeMat);

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
    renderSystem->Update(dt, camera, shaderProgram);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}