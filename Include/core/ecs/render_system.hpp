#pragma once

#include "coordinator.hpp"
#include "types.hpp"
#include "components.hpp"
#include "../camera.hpp"
#include <memory>
#include <utility>
#include <glad.h>
class RenderSystem;

struct OffscreenObjects
{
    std::unordered_map<std::string, unsigned int> framebuffers;
    std::unordered_map<std::string, unsigned int> textures;
    int fbWidth = 1000;
    int fbHeight = 1000;
};

class RenderModule
{
public:
    // optional fields if the module requires offscreen renders
    bool requiresOffscreenFrameBuffer = false;
    OffscreenObjects offscreenObjects;

    virtual void InitOffscreenFramebuffers() {}

    virtual void RenderOffscreenFramebuffers(RenderSystem *renderSystem, const Camera &camera) {}

    virtual ~RenderModule()
    {
        for (auto &[name, tex] : offscreenObjects.textures)
            glDeleteTextures(1, &tex);
        for (auto &[name, fb] : offscreenObjects.framebuffers)
            glDeleteFramebuffers(1, &fb);
    };

    virtual std::pair<std::string, std::string> GetShaders(RenderSystem *renderSystem, Entity e) const = 0;

    // once an object
    virtual void UploadObjectUniforms(unsigned int program, RenderSystem *renderSystem, const Camera &camera, Entity entity) {}

    // once a mesh (multiple times an object)
    virtual void UploadMeshUniforms(unsigned int program, RenderSystem *renderSystem, Entity entity, int materialID) {}

    virtual void DrawObject(unsigned int program, RenderSystem *renderSystem, Entity e) {};
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

private:
    void RenderScene(float deltaTime, const Camera &camera);
};