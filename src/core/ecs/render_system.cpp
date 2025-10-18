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

            // Draw the model
            modelComponent.model->Draw();

            // Unbind texture
            if (texture)
                texture->unbind();
        }
    }
}