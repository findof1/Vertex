#include "animations/animations_system.hpp"
#include "core/material.hpp"
#include "core/model.hpp"
#include <glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "core/texture.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include "animations/animation.hpp"

void AnimationsSystem::Init(std::shared_ptr<Coordinator> coordinator)
{
    gCoordinator = coordinator;
}

void AnimationsSystem::Update(float deltaTime, const Camera &camera)
{
    for (auto const &entity : mEntities)
    {
        if (!gCoordinator->HasComponent<AnimationComponent>(entity) || !gCoordinator->HasComponent<AnimatedModelComponent>(entity))
        {
            continue;
        }

        auto &animComp = gCoordinator->GetComponent<AnimationComponent>(entity);
        auto &animModelComp = gCoordinator->GetComponent<AnimatedModelComponent>(entity);

        if (!animComp.animation || !animComp.playing)
        {
            animModelComp.model->UnbindAnimation();
            continue;
        }

        animComp.currentTime += deltaTime;

        float duration = animComp.animation->GetDuration();
        if (duration > 0.0f)
        {
            animComp.currentTime = fmod(animComp.currentTime, duration);
        }

        animComp.animation->Update(deltaTime);

        for (int i = 0; i < animModelComp.model->boneInfo.size(); i++)
        {
            animComp.animation->finalBoneMatrices->at(i) = animComp.animation->finalBoneMatrices->at(i) * animModelComp.model->boneInfo.at(i).offsetMatrix;
        }

        animModelComp.model->BindAnimation(animComp.animation->GetFinalBoneMatrices());
    }
}
