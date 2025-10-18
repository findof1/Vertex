#include "core/render_system.hpp"
#include "core/material.hpp"
#include "core/model.hpp"
#include <glad.h>
#include <glm/gtc/type_ptr.hpp>

void RenderSystem::Init(std::shared_ptr<Coordinator> coordinator) {
    gCoordinator = coordinator;
}

void RenderSystem::Update(float deltaTime, const Camera& camera, unsigned int shaderProgram) {
    glUseProgram(shaderProgram);

    // Set view and projection matrices
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(16.0f / 12.0f); // Aspect ratio

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Render all entities with ModelComponent
    for (auto const& entity : mEntities) {
        auto& transform = gCoordinator->GetComponent<TransformComponent>(entity);
        auto& modelComponent = gCoordinator->GetComponent<ModelComponent>(entity);

        if (modelComponent.model) {
            // Set model matrix
            glm::mat4 model = transform.GetMatrix();
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            // Set material color if available
            if (gCoordinator->HasComponent<MaterialComponent>(entity)) {
                auto& material = gCoordinator->GetComponent<MaterialComponent>(entity);
                if (material.material) {
                    glUniform3fv(glGetUniformLocation(shaderProgram, "materialColor"), 1, glm::value_ptr(material.material->getAlbedo()));
                }
            }
            else {
                // Default white color
                glUniform3f(glGetUniformLocation(shaderProgram, "materialColor"), 1.0f, 1.0f, 1.0f);
            }

            // Draw the model
            modelComponent.model->Draw();
        }
    }
}