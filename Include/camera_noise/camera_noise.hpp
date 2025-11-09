#include "core/ecs/render_system.hpp"
#include <GLFW/glfw3.h>

class CameraNoisePass : public PostProcessPass
{
public:
  CameraNoisePass()
  {
    shaderVert = "shaders/basic_materials/quad.vert";
    shaderFrag = "shaders/camera_noise/camera_noise.frag";
  }

  virtual void UploadUniforms(unsigned int program) override
  {
    glUniform1f(glGetUniformLocation(program, "time"), glfwGetTime());
    glUniform1f(glGetUniformLocation(program, "noiseAmount"), 0.05);
  }
};