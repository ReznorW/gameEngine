#pragma once

#include <glm/glm.hpp>

// Forward declarations
class Camera;
class Shader;

struct OBB {
    glm::vec3 center;
    glm::vec3 extents;
    glm::mat3 axes;

    OBB() : center(0.0f), extents(1.0f), axes(1.0f) {}
    OBB(const glm::vec3& min, const glm::vec3& max);
};

void drawOBB(const OBB& obb, const Camera& camera, Shader* debugShader, const glm::vec3& color);