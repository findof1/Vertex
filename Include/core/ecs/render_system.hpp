#pragma once

#include "coordinator.hpp"
#include "types.hpp"
#include "components.hpp"
#include "../camera.hpp"
#include <memory>

class RenderSystem : public System
{
public:
    void Init(std::shared_ptr<Coordinator> coordinator);
    void Update(float deltaTime, const Camera &camera, unsigned int shaderProgram);

private:
    std::shared_ptr<Coordinator> gCoordinator;
};