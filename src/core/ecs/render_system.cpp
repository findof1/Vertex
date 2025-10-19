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

void RenderSystem::Init(std::shared_ptr<Coordinator> coordinator)
{
    gCoordinator = coordinator;
}

void RenderSystem::Update(float deltaTime, const Camera &camera)
{
    unsigned int shaderProgram = GetOrCreateShader("shaders/basic_materials/shader.vert", "shaders/basic_materials/shader.frag");
    glUseProgram(shaderProgram);

    // Set view and projection matrices
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(16.0f / 12.0f); // Aspect ratio

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Render all entities with ModelComponent
    for (auto const &entity : mEntities)
    {
        auto &transform = gCoordinator->GetComponent<TransformComponent>(entity);
        auto &modelComponent = gCoordinator->GetComponent<ModelComponent>(entity);

        if (modelComponent.model)
        {
            // Set model matrix
            glm::mat4 model = transform.GetMatrix();
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            // Material
            glm::vec3 albedo(1.0f, 1.0f, 1.0f); // default
            std::shared_ptr<Texture> texture = nullptr;

            // Set material albedo if available
            if (gCoordinator->HasComponent<MaterialComponent>(entity))
            {
                auto &materialComponent = gCoordinator->GetComponent<MaterialComponent>(entity);
                if (materialComponent.material)
                {
                    albedo = materialComponent.material->getAlbedo();

                    if (materialComponent.material->hasTexture())
                        texture = materialComponent.material->getTexture();
                }
            }

            glUniform3fv(glGetUniformLocation(shaderProgram, "materialAlbedo"), 1, glm::value_ptr(albedo));

            // Bind the texture
            if (texture)
            {
                unsigned int slot = 0; // Use slot 0 for base textures
                texture->bind(slot);
                glUniform1i(glGetUniformLocation(shaderProgram, "albedoMap"), slot);
                glUniform1i(glGetUniformLocation(shaderProgram, "useAlbedoMap"), true);
            }
            else
            {
                glUniform1i(glGetUniformLocation(shaderProgram, "useAlbedoMap"), false);
            }

            // Lighting
            // This if statement ignores lighting for all entities with a light component
            if (!gCoordinator->HasComponent<PointLightComponent>(entity))
            {
                glUniform1i(glGetUniformLocation(shaderProgram, "ignoreLighting"), false);

                int lightCount = 0;
                for (auto const &entityLight : mEntities)
                {
                    if (!(gCoordinator->HasComponent<PointLightComponent>(entityLight) && gCoordinator->HasComponent<TransformComponent>(entityLight)))
                        continue;

                    auto &lightTransform = gCoordinator->GetComponent<TransformComponent>(entityLight);
                    auto &lightComponent = gCoordinator->GetComponent<PointLightComponent>(entityLight);

                    if (lightCount >= 64)
                        break;

                    std::string base = "pointLights[" + std::to_string(lightCount) + "]";

                    glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".position").c_str()), 1, glm::value_ptr(lightTransform.translation));
                    glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".color").c_str()), 1, glm::value_ptr(lightComponent.color));
                    glUniform1f(glGetUniformLocation(shaderProgram, (base + ".intensity").c_str()), lightComponent.intensity);
                    glUniform1f(glGetUniformLocation(shaderProgram, (base + ".constant").c_str()), lightComponent.constant);
                    glUniform1f(glGetUniformLocation(shaderProgram, (base + ".linear").c_str()), lightComponent.linear);
                    glUniform1f(glGetUniformLocation(shaderProgram, (base + ".quadratic").c_str()), lightComponent.quadratic);

                    lightCount++;
                }

                glUniform1i(glGetUniformLocation(shaderProgram, "numPointLights"), lightCount);

                // Pass camera pos to fragment shader for specular highlights
                glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.Position));
            }
            else
            {
                glUniform1i(glGetUniformLocation(shaderProgram, "ignoreLighting"), true);
            }

            // Draw the model
            modelComponent.model->Draw();
            // Unbind texture
            if (texture)
                texture->unbind();
        }
    }
}