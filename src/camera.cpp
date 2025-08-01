#include "camera.hpp"

// === Constructor ===
Camera::Camera(float aspect)
    : position(glm::vec3(0.0f, 0.0f, 3.0f)), worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), fov(80.0f), aspect(aspect), near(0.1f), far(100.0f), yaw(-90.0f), pitch(0.0f), roll(0.0f) {
    updateCameraVectors();
}

// === Getters ===
glm::vec3 Camera::getPosition() const {return position;}
glm::vec3 Camera::getFront() const {return front;}
glm::vec3 Camera::getRight() const {return right;}
glm::vec3 Camera::getUp() const {return up;}
glm::vec3 Camera::getWorldUp() const {return worldUp;}
float Camera::getFOV() const {return fov;}
float Camera::getAspectRatio() const {return aspect;}
float Camera::getNear() const {return near;}
float Camera::getFar() const {return far;}
float Camera::getYaw() const {return yaw;}
float Camera::getPitch() const {return pitch;}
float Camera::getRoll() const {return roll;}

// === Setters ===
void Camera::setPosition(const glm::vec3& newPosition) {position = newPosition;}
void Camera::setFront(const glm::vec3& newFront) {front = newFront;}
void Camera::setRight(const glm::vec3& newRight) {right = newRight;}
void Camera::setUp(const glm::vec3& newUp) {up = newUp;}
void Camera::setWorldUp(const glm::vec3& newWorldUp) {worldUp = newWorldUp;}
void Camera::setFOV(const float& newFov) {fov = newFov;}
void Camera::setAspectRatio(const float& newAspectRatio) {aspect = newAspectRatio;}
void Camera::setNear(const float& newNear) {near = newNear;}
void Camera::setFar(const float& newFar) {far = newFar;}
void Camera::setYaw(const float& newYaw) {yaw = newYaw;}
void Camera::setPitch(const float& newPitch) {pitch = newPitch;}
void Camera::setRoll(const float& newRoll) {roll = newRoll;}

// === Projection handling ===
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), aspect, near, far);
}

glm::mat4 Camera::getProjectionMatrix(const float nearPlane, const float farPlane) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

// === Camera controllers ===
void Camera::move(const glm::vec3& direction, const float& speed) {
    position.x += direction.x * speed;
    position.z += direction.z * speed;
}

void Camera::moveVert(const glm::vec3& direction, const float& speed) {
    position += direction * speed;
}

void Camera::rotate(glm::vec3& rotation) {
    // Apply rotation
    yaw += rotation.y;
    pitch += rotation.x;
    roll += rotation.z;

    // Build rotation matrix (rot)
    glm::mat4 rot = glm::mat4(1.0f);
    rot = glm::rotate(rot, glm::radians(yaw),   glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw: Y axis
    rot = glm::rotate(rot, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch: X axis
    rot = glm::rotate(rot, glm::radians(roll),  glm::vec3(0.0f, 0.0f, 1.0f)); // Roll: Z axis

    // Make new vectors
    glm::vec4 direction = rot * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    front = glm::normalize(glm::vec3(direction));
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

std::vector<glm::vec4> Camera::getFrustumCornersWorldSpace(const float nearPlane, const float farPlane) {
    const auto inv = glm::inverse(getProjectionMatrix(nearPlane, farPlane) * getViewMatrix());

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; x++) {
        for (unsigned int y = 0; y < 2; y++) {
            for (unsigned int z = 0; z < 2; z++) {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

glm::mat4 Camera::getLightSpaceMatrix(const float nearPlane, const float farPlane, const glm::vec3 lightDir) {
    const auto corners = getFrustumCornersWorldSpace(nearPlane, farPlane);

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners) {
        center += glm::vec3(v);
    }
    center /= corners.size();

    const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& v : corners) {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    float zPadding = 10.0f;
    minZ -= zPadding;
    maxZ += zPadding;

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
}

std::vector<glm::mat4> Camera::getLightSpaceMatrices(const glm::vec3 lightDir, std::vector<float> shadowCascadeLevels) {
    std::vector<glm::mat4> result;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; i++) {
        if (i == 0) {
            result.push_back(getLightSpaceMatrix(near, shadowCascadeLevels[i], lightDir));
        } else if (i < shadowCascadeLevels.size()) {
            result.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i], lightDir));
        } else {
            result.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], far, lightDir));
        }
    }
    return result;
}
