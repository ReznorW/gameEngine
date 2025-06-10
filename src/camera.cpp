#include "camera.hpp"

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
