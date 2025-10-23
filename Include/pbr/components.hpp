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
    std::vector<std::shared_ptr<PBRMaterial>> materials;

    PBRMaterialComponent() = default;
    PBRMaterialComponent(std::shared_ptr<PBRMaterial> m) { materials.push_back(m); }
};
