#pragma once

#include <memory>
#include <glm/glm.hpp>

// Forward declarations
class RigidBody;
class Collider;

struct ColliderComponent {
    std::shared_ptr<Collider> collider;

    ColliderComponent() = default;
    explicit ColliderComponent(std::shared_ptr<Collider> c) : collider(std::move(c)) {}
};

struct RigidbodyComponent {
    std::shared_ptr<RigidBody> rigidBody;

    RigidbodyComponent() = default;
    explicit RigidbodyComponent(std::shared_ptr<RigidBody> rb) : rigidBody(std::move(rb)) {}
};