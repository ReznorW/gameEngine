#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <limits>
#include <iostream>
#include <ostream>

#include "obb.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "object.hpp"
#include "resources.hpp"

// === Constructor ===
OBB::OBB(const glm::vec3& min, const glm::vec3& max) 
    : center((min + max) * 0.5f), extents(max - center), axes(glm::mat3(1.0f)) {}

// === Collision detection ===
bool areIntersecting(Object& objA, Object& objB) {
    const float EPSILON = 1e-5f;

    OBB& a = objA.obb;
    OBB& b = objB.obb;

    // Rotation matrix expressing b in a's frame
    glm::mat3 R;
    glm::mat3 AbsR;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            R[i][j] = glm::dot(a.axes[i], b.axes[j]);
            AbsR[i][j] = std::abs(R[i][j]) + EPSILON;
        }
    }

    // Vector between centers in a's frame
    glm::vec3 t = b.center - a.center;
    t = glm::vec3(glm::dot(t, a.axes[0]), glm::dot(t, a.axes[1]), glm::dot(t, a.axes[2]));

    // Test axes L = A0, A1, A2
    for (int i = 0; i < 3; ++i) {
        float ra = a.extents[i];
        float rb = b.extents[0] * AbsR[i][0] + b.extents[1] * AbsR[i][1] + b.extents[2] * AbsR[i][2];
        if (std::abs(t[i]) > ra + rb) {
            return false;
        }
    }

    // Test axes L = B0, B1, B2
    for (int i = 0; i < 3; ++i) {
        float ra = a.extents[0] * AbsR[0][i] + a.extents[1] * AbsR[1][i] + a.extents[2] * AbsR[2][i];
        float rb = b.extents[i];
        if (std::abs(t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i]) > ra + rb) {
            return false;
        }
    }

    // Test axis L = Ai x Bj
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            float ra = a.extents[(i + 1) % 3] * AbsR[(i + 2) % 3][j] + a.extents[(i + 2) % 3] * AbsR[(i + 1) % 3][j];
            float rb = b.extents[(j + 1) % 3] * AbsR[i][(j + 2) % 3] + b.extents[(j + 2) % 3] * AbsR[i][(j + 1) % 3];

            float tVal = t[(i + 2) % 3] * R[(i + 1) % 3][j] - t[(i + 1) % 3] * R[(i + 2) % 3][j];
            if (std::abs(tVal) > ra + rb) {
                return false;
            }
        }
    }

    return true;
}

bool getMinimumTranslationVector(const OBB& a, const OBB& b, glm::vec3& mtvAxis, float& mtvDistance) {
    const float EPSILON = 1e-4f;
    glm::vec3 axes[15];
    int axisCount = 0;

    // Primary axes
    for (int i = 0; i < 3; ++i) {
        axes[axisCount++] = a.axes[i];
    }
    for (int i = 0; i < 3; ++i) {
        axes[axisCount++] = b.axes[i];
    }

    // Cross products of edge directions
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            glm::vec3 cross = glm::cross(a.axes[i], b.axes[j]);
            if (glm::length2(cross) > EPSILON) {
                axes[axisCount++] = glm::normalize(cross);
            }
        }

    float minOverlap = std::numeric_limits<float>::max();
    glm::vec3 bestAxis;

    for (int i = 0; i < axisCount; ++i) {
        glm::vec3 axis = axes[i];

        // Project both OBBs onto this axis
        float minA, maxA, minB, maxB;
        projectOntoAxis(a, axis, minA, maxA);
        projectOntoAxis(b, axis, minB, maxB);

        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
        if (overlap < 0.0f) {
            return false;
        }

        if (overlap < minOverlap) {
            minOverlap = overlap;
            bestAxis = axis;
        }
    }

    mtvAxis = bestAxis;
    mtvDistance = minOverlap;
    return true;
}

void projectOntoAxis(const OBB& obb, const glm::vec3& axis, float& min, float& max) {
    float center = glm::dot(obb.center, axis);
    float radius = std::abs(glm::dot(obb.axes[0] * obb.extents.x, axis)) + std::abs(glm::dot(obb.axes[1] * obb.extents.y, axis)) + std::abs(glm::dot(obb.axes[2] * obb.extents.z, axis));

    min = center - radius;
    max = center + radius;
}

void resolveCollision(Object& objA, Object& objB) {
    objB.updateOBB();

    glm::vec3 mtvAxis;
    float mtvDist;

    if (!getMinimumTranslationVector(objA.obb, objB.obb, mtvAxis, mtvDist)) {
        return;
    }

    glm::vec3 centerDelta = objA.obb.center - objB.obb.center;
    if (glm::dot(centerDelta, mtvAxis) < 0.0f) {
        mtvAxis = -mtvAxis;
    }

    glm::vec3 correction = mtvAxis * mtvDist;

    if (glm::length2(correction) > 0.0f) {
        if (objA.isMoveable && objB.isMoveable) {
            glm::vec3 half = correction * 0.5f;
            objA.transform.position += half;
            objB.transform.position -= half;
            objA.updateOBB();
            objB.updateOBB();
        } else if (objA.isMoveable) {
            objA.transform.position += correction;
            objA.updateOBB();
        } else if (objB.isMoveable) {
            objB.transform.position -= correction;
            objB.updateOBB();
        }
    }

    if (correction.y > 0.0f) {
        objA.isGrounded = true;
        objA.transform.velocity.y = 0.0f;
    }
}