#pragma once
#include "core/ecs/system.hpp"
#include "core/ecs/components.hpp"
#include "core/ecs/coordinator.hpp"
#include "physics/components.hpp"
#include "physics/rigidbody.hpp"
#include <memory>

class PhysicsSystem : public System {
public:
    void Update(std::shared_ptr<Coordinator> coordinator, float deltaTime) {
        for (auto const& entity : mEntities) {
            auto& transform = coordinator->GetComponent<TransformComponent>(entity);
            auto& rigidComp = coordinator->GetComponent<RigidbodyComponent>(entity);

            auto rb = rigidComp.rigidBody;
            if (!rb) continue;

            rb->ApplyGravity(deltaTime);
            rb->Integrate(deltaTime, transform.translation, transform.rotation);
        }
    }
};