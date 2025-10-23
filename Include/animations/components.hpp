#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class AnimatedModel;
class Animation;

struct AnimatedModelComponent
{
    std::shared_ptr<AnimatedModel> model;

    AnimatedModelComponent() = default;
    AnimatedModelComponent(std::shared_ptr<AnimatedModel> m) : model(m) {}
};

struct AnimationComponent
{
    std::shared_ptr<Animation> animation;
    float currentTime = 0.0f;
    bool playing = true;

    AnimationComponent() = default;
    AnimationComponent(std::shared_ptr<Animation> anim)
        : animation(anim) {}
};