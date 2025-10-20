#pragma once

#include "coordinator.hpp"
#include "types.hpp"
#include "components.hpp"
#include "../camera.hpp"
#include <memory>
#include <utility>
class RenderSystem;

class RenderModule
{
public:
    virtual ~RenderModule() = default;

    virtual std::pair<std::string, std::string> GetShaders(RenderSystem *renderSystem, Entity e) const = 0;

    virtual void UploadUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity entity) {}

    virtual void DrawObject(RenderSystem *renderSystem, Entity e) {};
};

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
    std::shared_ptr<Coordinator> gCoordinator;
    std::vector<std::unique_ptr<RenderModule>> modules;

    unsigned int GetOrCreateShader(const std::string &vert, const std::string &frag);

    void AddModule(std::unique_ptr<RenderModule> module);
    void Init(std::shared_ptr<Coordinator> coordinator);
    void Update(float deltaTime, const Camera &camera);
};