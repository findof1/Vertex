#include "core/ecs/render_system.hpp"
#include "core/material.hpp"
#include "core/model.hpp"
#include <glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "core/texture.hpp"

void RenderSystem::Init(std::shared_ptr<Coordinator> coordinator)
{
    gCoordinator = coordinator;
}

void RenderSystem::Update(float deltaTime, const Camera &camera, unsigned int shaderProgram)
{
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