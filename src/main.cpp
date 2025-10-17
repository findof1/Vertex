#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "core/camera.hpp"
#include <vector>

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

std::vector<float> vertices = {
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.3f, 0.7f, 0.2f};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0,
    4, 5, 6,
    6, 7, 4,
    0, 1, 5,
    5, 4, 0,
    2, 3, 7,
    7, 6, 2,
    0, 3, 7,
    7, 4, 0,
    1, 2, 6,
    6, 5, 1};

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
  if (!glfwInit())
    return -1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", NULL, NULL);
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

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_DEPTH_TEST);

  unsigned int VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  unsigned int shaderProgram = createShaderProgram(
      "shaders/shader.vert",
      "shaders/shader.frag");

  float lastTime = glfwGetTime();
  while (!glfwWindowShouldClose(window))
  {
    float time = glfwGetTime();
    float dt = time - lastTime;
    lastTime = glfwGetTime();

    processInput(window, dt);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 proj = camera.getProjectionMatrix((float)WIDTH / (float)HEIGHT);
    glm::mat4 view = camera.getViewMatrix();
    {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
      model = glm::scale(model, glm::vec3(1, 1, 1));

      glUseProgram(shaderProgram);
      int locModel = glGetUniformLocation(shaderProgram, "model");
      glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

      int locView = glGetUniformLocation(shaderProgram, "view");
      glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));

      int locProjection = glGetUniformLocation(shaderProgram, "projection");
      glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(proj));

      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, 120, GL_UNSIGNED_INT, 0);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
