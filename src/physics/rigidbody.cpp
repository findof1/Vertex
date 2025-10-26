#include "physics/rigidbody.hpp"

void RigidBody::ApplyGravity(float deltaTime) {
    if (useGravity) velocity += gravity * deltaTime;
}

void RigidBody::Integrate(float deltaTime, glm::vec3& position, glm::vec3& rotation) {
    // Apply accumulated forces
    glm::vec3 acceleration = forces / mass;
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;
    rotation += angularVelocity * deltaTime;

    // Reset forces each frame
    forces = glm::vec3(0.0f);
}