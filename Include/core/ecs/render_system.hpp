#pragma once

#include "coordinator.hpp"
#include "types.hpp"
#include "components.hpp"
#include "../camera.hpp"
#include <memory>

struct ShaderKey
{
    std::string vertex;
    std::string fragment;
    bool operator==(const ShaderKey &other) const
    {
        return vertex == other.vertex && fragment == other.fragment;
    }
};

struct ShaderKeyHash
{
    std::size_t operator()(const ShaderKey &k) const noexcept
    {
        return std::hash<std::string>()(k.vertex) ^ (std::hash<std::string>()(k.fragment) << 1);
    }
};

class RenderSystem : public System
{
public:
    std::unordered_map<ShaderKey, unsigned int, ShaderKeyHash> shaderCache;

    unsigned int GetOrCreateShader(const std::string &vert, const std::string &frag);

    void Init(std::shared_ptr<Coordinator> coordinator);
    void Update(float deltaTime, const Camera &camera);

private:
    std::shared_ptr<Coordinator> gCoordinator;
};