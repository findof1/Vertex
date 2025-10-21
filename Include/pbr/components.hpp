#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class PBRMaterial;

struct PBRMaterialComponent
{
    std::shared_ptr<PBRMaterial> material;

    PBRMaterialComponent() = default;
    PBRMaterialComponent(std::shared_ptr<PBRMaterial> m) : material(m) {}
};
