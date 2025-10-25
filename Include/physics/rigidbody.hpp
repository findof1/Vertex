#pragma once
#include <glm/glm.hpp>

class RigidBody {
public:
    glm::vec3 velocity{0.0f};
    glm::vec3 angularVelocity{0.0f};
    glm::vec3 forces{0.0f};
    glm::vec3 gravity{0.0f, -9.81f, 0.0f};
    bool useGravity = true;
    float mass = 1.0f;

    RigidBody() = default;
    ~RigidBody() = default;

    void ApplyForce(const glm::vec3& force) { forces += force; }

    void ApplyGravity(float deltaTime);
    void Integrate(float deltaTime, glm::vec3& position, glm::vec3& rotation);
};