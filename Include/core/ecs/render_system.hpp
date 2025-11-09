#pragma once

#include "coordinator.hpp"
#include "types.hpp"
#include "components.hpp"
#include "../camera.hpp"
#include <memory>
#include <utility>
#include <glad.h>
#include <functional>
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

    virtual void RenderOffscreenFramebuffers(RenderSystem *renderSystem, float deltaTime, const Camera &camera) {}

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

class PostProcessPass
{
public:
    std::string shaderVert;
    std::string shaderFrag;
    virtual unsigned int DoSubPasses(RenderSystem *renderSystem, unsigned int inputTex, int &passIndex)
    {
        return inputTex;
    };
    virtual void UploadUniforms(unsigned int program) {};
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
    std::vector<std::unique_ptr<PostProcessPass>> postProcessPasses;
    int screenWidth;
    int screenHeight;

    unsigned int sceneFBO, sceneColorTex, sceneDepthRBO;
    unsigned int pingpongFBO[2], pingpongColorTex[2];
    unsigned int quadVAO, quadVBO;

    unsigned int GetOrCreateShader(const std::string &vert, const std::string &frag);

    void AddModule(std::unique_ptr<RenderModule> module);
    void Init(std::shared_ptr<Coordinator> coordinator, int screenWidth, int screenHeight);
    void InitPostProcessing();
    void Update(float deltaTime, const Camera &camera);
    void RenderScene(float deltaTime, const Camera &camera, bool mainRender = true, bool useClippingPlane = false, glm::vec4 clippingPlane = glm::vec4(-1));

    // returns image that results from pass, passIndex is the number of passes that have already happened in the frame
    unsigned int DoPostProcessPass(std::unique_ptr<PostProcessPass> &pass, unsigned int inputTex, int &passIndex);
};