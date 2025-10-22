#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class AnimatedModel;

struct AnimatedModelComponent
{
    std::shared_ptr<AnimatedModel> model;

    AnimatedModelComponent() = default;
    AnimatedModelComponent(std::shared_ptr<AnimatedModel> m) : model(m) {}
};