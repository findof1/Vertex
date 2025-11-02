#include "core/ecs/render_system.hpp"
#include "core/material.hpp"
#include "core/model.hpp"
#include <glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "core/texture.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

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

unsigned int RenderSystem::GetOrCreateShader(const std::string &vert, const std::string &frag)
{
    ShaderKey key{vert, frag};
    auto it = shaderCache.find(key);
    if (it != shaderCache.end())
        return it->second;

    unsigned int program = createShaderProgram(vert.c_str(), frag.c_str());
    shaderCache[key] = program;
    return program;
}

void RenderSystem::Init(std::shared_ptr<Coordinator> coordinator, int screenWidth, int screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    gCoordinator = coordinator;
    glEnable(GL_CLIP_DISTANCE0);
}

void RenderSystem::AddModule(std::unique_ptr<RenderModule> module)
{
    for (const auto &m : modules)
    {
        if (typeid(*m) == typeid(*module))
            return;
    }
    if (module->requiresOffscreenFrameBuffer)
    {
        module->InitOffscreenFramebuffers();
    }

    modules.push_back(std::move(module));
}

void RenderSystem::Update(float deltaTime, const Camera &camera)
{
    for (auto &module : modules)
    {
        if (!module->requiresOffscreenFrameBuffer)
        {
            continue;
        }

        module->RenderOffscreenFramebuffers(this, deltaTime, camera);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default framebuffer
    glViewport(0, 0, screenWidth, screenHeight);

    RenderScene(deltaTime, camera);
}

void RenderSystem::RenderScene(float deltaTime, const Camera &camera, bool mainRender, bool useClippingPlane, glm::vec4 clippingPlane)
{

    for (auto const &entity : mEntities)
    {
        std::string vertexPath, fragmentPath;

        for (auto &module : modules)
        {
            if (!mainRender && module->requiresOffscreenFrameBuffer)
            {
                continue;
            }
            auto [vert, frag] = module->GetShaders(this, entity);
            if (!vert.empty())
                vertexPath = vert;
            if (!frag.empty())
                fragmentPath = frag;
        }

        if (vertexPath.empty() || fragmentPath.empty())
        {
            continue;
        }

        unsigned int program = GetOrCreateShader(vertexPath, fragmentPath);
        glUseProgram(program);

        if (useClippingPlane)
        {
            glUniform1i(glGetUniformLocation(program, "enableClip"), GL_TRUE);
            glUniform4f(glGetUniformLocation(program, "clipPlane"), clippingPlane.x, clippingPlane.y, clippingPlane.z, clippingPlane.w);
        }
        else
        {
            glUniform1i(glGetUniformLocation(program, "enableClip"), GL_FALSE);
        }

        for (auto &module : modules)
        {
            if (!mainRender && module->requiresOffscreenFrameBuffer)
            {
                continue;
            }
            module->UploadObjectUniforms(program, this, camera, entity);
        }

        for (auto &module : modules)
        {
            if (!mainRender && module->requiresOffscreenFrameBuffer)
            {
                continue;
            }
            module->DrawObject(program, this, entity);
        }

        int maxUnits = 0;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
        for (int i = 0; i < maxUnits; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glActiveTexture(GL_TEXTURE0);
    }
}