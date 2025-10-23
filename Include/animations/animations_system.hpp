#pragma once

#include "core/ecs/coordinator.hpp"
#include "core/ecs/types.hpp"
#include "core/ecs/components.hpp"
#include "components.hpp"
#include "core/camera.hpp"
#include <memory>
#include <utility>

class AnimationsSystem : public System
{
public:
    std::shared_ptr<Coordinator> gCoordinator;

    void Init(std::shared_ptr<Coordinator> coordinator);
    void Update(float deltaTime, const Camera &camera);
};