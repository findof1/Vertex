#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class WaterMesh;

struct WaterMeshComponent
{
    std::shared_ptr<WaterMesh> water;

    WaterMeshComponent() = default;
    WaterMeshComponent(std::shared_ptr<WaterMesh> waterMesh) : water(waterMesh) {}
};