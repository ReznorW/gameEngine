#pragma once

#include <glm/glm.hpp>

// Forward declarations
class Camera;
class Shader;
class Object;

struct OBB {
    glm::vec3 center;
    glm::vec3 extents;
    glm::mat3 axes;

    OBB() : center(0.0f), extents(1.0f), axes(1.0f) {}
    OBB(const glm::vec3& min, const glm::vec3& max);
};

bool areIntersecting(Object& objA, Object& objB);
bool getMinimumTranslationVector(const OBB& a, const OBB& b, glm::vec3& mtvAxis, float& mtvDistance);
void projectOntoAxis(const OBB& obb, const glm::vec3& axis, float& min, float& max);
void resolveCollision(Object& objA, Object& objB);
