#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Camera definition
struct Camera {
    // Camera state
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp;
    float fov;
    float aspect;
    float near;
    float far;
    float yaw;
    float pitch;
    float roll;

    // Getters
    glm::vec3 getPosition() const;
    glm::vec3 getFront() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;
    glm::vec3 getWorldUp() const;
    float getFOV() const;
    float getAspectRatio() const;
    float getNear() const;
    float getFar() const;
    float getYaw() const;
    float getPitch() const;
    float getRoll() const;

    // Setters
    void setPosition(const glm::vec3& newPosition);
    void setFront(const glm::vec3& newFront);
    void setRight(const glm::vec3& newRight);
    void setUp(const glm::vec3& newUp);
    void setWorldUp(const glm::vec3& newWorldUp);
    void setFOV(const float& newFov);
    void setAspectRatio(const float& newAspectRatio);
    void setNear(const float& newNear);
    void setFar(const float& newFar);
    void setYaw(const float& newYaw);
    void setPitch(const float& newPitch);
    void setRoll(const float& newRoll);

    // Projection handling
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    // Camera controllers
    void move(const glm::vec3& direction, const float& speed);
    void moveVert(const glm::vec3& direction, const float& speed);
    void rotate(glm::vec3& rotation);
    void updateCameraVectors();
};
