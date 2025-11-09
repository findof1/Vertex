#include "core/ecs/render_system.hpp"

class BloomExtractSubPass : public PostProcessPass
{
public:
  BloomExtractSubPass()
  {
    shaderVert = "shaders/basic_materials/quad.vert";
    shaderFrag = "shaders/bloom/bloom_extract.frag";
  }

  virtual void UploadUniforms(unsigned int program) override
  {
    glUniform1f(glGetUniformLocation(program, "threshold"), 0.8);
  }

  virtual unsigned int DoSubPasses(RenderSystem *renderSystem, unsigned int inputTex, int &passIndex) override
  {
    return inputTex;
  }
};

class BloomBlurHSubPass : public PostProcessPass
{
public:
  int WIDTH, HEIGHT;

  BloomBlurHSubPass(int WIDTH, int HEIGHT) : WIDTH(WIDTH), HEIGHT(HEIGHT)
  {
    shaderVert = "shaders/basic_materials/quad.vert";
    shaderFrag = "shaders/bloom/bloom_blur.frag";
  }

  virtual void UploadUniforms(unsigned int program) override
  {
    glUniform1f(glGetUniformLocation(program, "texelWidth"), 1.0f / WIDTH);
    glUniform1f(glGetUniformLocation(program, "texelHeight"), 1.0f / HEIGHT);
    glUniform1i(glGetUniformLocation(program, "horizontal"), 1);
    glUniform1f(glGetUniformLocation(program, "blurScale"), 1.0f);
  }

  virtual unsigned int DoSubPasses(RenderSystem *renderSystem, unsigned int inputTex, int &passIndex) override
  {
    return inputTex;
  }
};

class BloomBlurVSubPass : public PostProcessPass
{
public:
  int WIDTH, HEIGHT;
  BloomBlurVSubPass(int WIDTH, int HEIGHT) : WIDTH(WIDTH), HEIGHT(HEIGHT)
  {
    shaderVert = "shaders/basic_materials/quad.vert";
    shaderFrag = "shaders/bloom/bloom_blur.frag";
  }

  virtual void UploadUniforms(unsigned int program) override
  {
    glUniform1f(glGetUniformLocation(program, "texelWidth"), 1.0f / WIDTH);
    glUniform1f(glGetUniformLocation(program, "texelHeight"), 1.0f / HEIGHT);
    glUniform1i(glGetUniformLocation(program, "horizontal"), 0);
    glUniform1f(glGetUniformLocation(program, "blurScale"), 1.0f);
  }

  virtual unsigned int DoSubPasses(RenderSystem *renderSystem, unsigned int inputTex, int &passIndex) override
  {
    return inputTex;
  }
};

class BloomPass : public PostProcessPass
{
public:
  int WIDTH, HEIGHT;
  BloomPass(int WIDTH, int HEIGHT) : WIDTH(WIDTH), HEIGHT(HEIGHT)
  {
    glGenTextures(1, &bloomTexture);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    shaderVert = "shaders/basic_materials/quad.vert";
    shaderFrag = "shaders/bloom/bloom_combine.frag";
    subpasses.push_back(std::make_unique<BloomExtractSubPass>());
    subpasses.push_back(std::make_unique<BloomBlurHSubPass>(WIDTH, HEIGHT));
    subpasses.push_back(std::make_unique<BloomBlurVSubPass>(WIDTH, HEIGHT));
  }

  std::vector<std::unique_ptr<PostProcessPass>> subpasses;

  unsigned int bloomTexture;

  virtual void UploadUniforms(unsigned int program) override
  {
    glUniform1f(glGetUniformLocation(program, "bloomStrength"), 0.6);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glUniform1i(glGetUniformLocation(program, "bloomTexture"), 1);
  }

  virtual unsigned int DoSubPasses(RenderSystem *renderSystem, unsigned int inputTex, int &passIndex) override
  {
    unsigned int extractTexture = renderSystem->DoPostProcessPass(subpasses[0], inputTex, passIndex);

    unsigned int prevImage = extractTexture;
    for (int i = 0; i < 5; i++)
    {
      prevImage = renderSystem->DoPostProcessPass(subpasses[1], prevImage, passIndex);
      prevImage = renderSystem->DoPostProcessPass(subpasses[2], prevImage, passIndex);
    }

    // do (passIndex - 1) % 2 because passIndex has since been incremented
    glBindFramebuffer(GL_READ_FRAMEBUFFER, renderSystem->pingpongFBO[(passIndex - 1) % 2]);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, WIDTH, HEIGHT);

    return inputTex;
  }
};