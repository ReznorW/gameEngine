#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <iostream>
#include <ostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "input.hpp"
#include "camera.hpp"
#include "window.hpp"
#include "scene.hpp"

// === Globals ===
bool mouseLookActive = false;
float lastX = 0.0f;
float lastY = 0.0f;
bool firstMouse = true;

float movementSpeed = 0.1f;
float lookSpeed = 0.1f;

bool Input::keys[512] = {false};
bool Input::mouseButtons[5] = {false};

// === Input processing ===
void Input::processInput(Window& window, Camera& camera, Scene& scene) {
    ImGuiIO& io = ImGui::GetIO();

    // Only process movement if ImGui doesn't want keyboard
    if (!io.WantCaptureKeyboard) {
        float currentSpeed = movementSpeed;
        
        // Speed boost when holding Left Control
        if (keys[GLFW_KEY_LEFT_CONTROL]) {
            currentSpeed *= 2.0f;
        }

        // Movement using stored key states
        if (keys[GLFW_KEY_ESCAPE]) {
            scene.clearSelection();
        }
        if (keys[GLFW_KEY_W]) {
            glm::vec3 forward = glm::normalize(glm::vec3(camera.getFront().x, 0.0f, camera.getFront().z));
            camera.move(forward, currentSpeed);
        }
        if (keys[GLFW_KEY_S]) {
            glm::vec3 backward = glm::normalize(glm::vec3(-camera.getFront().x, 0.0f, -camera.getFront().z));
            camera.move(backward, currentSpeed);
        }
        if (keys[GLFW_KEY_A]) {
            camera.move(-camera.getRight(), currentSpeed);
        }
        if (keys[GLFW_KEY_D]) {
            camera.move(camera.getRight(), currentSpeed);
        }
        if (keys[GLFW_KEY_SPACE]) {
            camera.moveVert(camera.getWorldUp(), currentSpeed);
        }
        if (keys[GLFW_KEY_LEFT_SHIFT]) {
            camera.moveVert(-camera.getWorldUp(), currentSpeed);
        }
        if (keys[GLFW_KEY_F1]) {
            if (camera.getFOV() < 135) {
                camera.setFOV(camera.getFOV() + lookSpeed);
            }
        }
        if (keys[GLFW_KEY_F2]) {
            if (camera.getFOV() > 20) {
                camera.setFOV(camera.getFOV() - lookSpeed);
            }
        }
    }
}

void Input::processMouseMovement(Camera& camera, float& xoffset, float& yoffset, bool constrainPitch) {
    // Set yaw
    camera.setYaw(camera.getYaw() + xoffset);

    // Set pitch
    if (constrainPitch) {
        float pitch = camera.getPitch() + yoffset;
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
        camera.setPitch(pitch);
    }

    // Update vectors
    camera.updateCameraVectors();
}

// === GLFW callbacks ===
void Input::mouse_button_callback(GLFWwindow* glfwWindow, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(glfwWindow, button, action, mods);

    if (button >= 0 && button < 5) {
        mouseButtons[button] = (action == GLFW_PRESS);
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }
    
    // Pass in context
    Context* context = static_cast<Context*>(glfwGetWindowUserPointer(glfwWindow));
    if (!context) {
         return;
    }

    Window& window = *context->window;
    Camera& camera = *context->camera;
    Scene& scene = *context->scene;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            mouseLookActive = true;
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // Get window size and center the mouse
            int width, height;
            glfwGetWindowSize(glfwWindow, &width, &height);
            double centerX = width / 2.0;
            double centerY = height / 2.0;
            glfwSetCursorPos(glfwWindow, centerX, centerY);

            // Initialize lastX and lastY to center to avoid jump
            lastX = static_cast<float>(centerX);
            lastY = static_cast<float>(centerY);

            firstMouse = true; // reset firstMouse so we don’t get a big jump
        } else if (action == GLFW_RELEASE) {
            mouseLookActive = false;
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double mouseX, mouseY;
            glfwGetCursorPos(window.getGLFWwindow(), &mouseX, &mouseY);

            int width, height;
            glfwGetWindowSize(window.getGLFWwindow(), &width, &height);

            glm::mat4 projection = camera.getProjectionMatrix();
            glm::mat4 view = camera.getViewMatrix();

            glm::vec3 ray = calculateRayFromMouse(mouseX, mouseY, width, height, projection, view);

            scene.clearSelection();
            
            float closestT = std::numeric_limits<float>::max();
            Object* selectedObject = nullptr;

            for (auto& obj : scene.getObjects()) {
                float t;
                if (RayIntersectsOBB(camera.getPosition(), ray, obj->obb, t)) {
                    if (t < closestT) {
                        closestT = t;
                        selectedObject = obj;
                    }
                }
            }

            if (selectedObject) {
                scene.selectObject(selectedObject->name);
            }
        }
    }
}

void Input::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)xpos, (float)ypos);
    
    if (!mouseLookActive) {
        return;
    }

    // Pass in context
    Context* context = static_cast<Context*>(glfwGetWindowUserPointer(window));
    if (!context) {
         return;
    }
    Camera& camera = *context->camera;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // inverted Y

    lastX = xpos;
    lastY = ypos;

    float sensitivity = lookSpeed; // tweak this value for rotation speed
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    Input::processMouseMovement(camera, xoffset, yoffset);
}

void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    
    if (key >= 0 && key < 512) {
        keys[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

void Input::char_callback(GLFWwindow* window, unsigned int c) {
    ImGui_ImplGlfw_CharCallback(window, c);
}

// === Raycasting utils ===
glm::vec3 calculateRayFromMouse(double mouseX, double mouseY, int screenWidth, int screenHeight, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
    // Step 1: Convert mouse position to Normalized Device Coordinates (NDC)
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight; // Note: Y is inverted
    float z = 1.0f;

    glm::vec3 rayNDS = glm::vec3(x, y, z);

    // Step 2: Convert NDC to Homogeneous Clip Coordinates
    glm::vec4 rayClip = glm::vec4(rayNDS.x, rayNDS.y, -1.0f, 1.0f);

    // Step 3: Convert to Eye Space
    glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Step 4: Convert to World Space
    glm::vec4 rayWorld = glm::inverse(viewMatrix) * rayEye;
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

    return rayDirection;
}

bool RayIntersectsOBB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const OBB& obb, float& t) {
    float tMin = -FLT_MAX;
    float tMax = FLT_MAX;
    glm::vec3 p = obb.center - rayOrigin;
    
    // Test against all three axes
    for (int i = 0; i < 3; i++) {
        glm::vec3 axis = obb.axes[i];
        float e = glm::dot(axis, p);
        float f = glm::dot(axis, rayDir);
        
        if (fabs(f) > 0.001f) {
            float t1 = (e + obb.extents[i]) / f;
            float t2 = (e - obb.extents[i]) / f;
            
            if (t1 > t2) std::swap(t1, t2);
            tMin = glm::max(tMin, t1);
            tMax = glm::min(tMax, t2);
            
            if (tMin > tMax) return false;
            if (tMax < 0) return false;
        }
        else if (-e - obb.extents[i] > 0 || -e + obb.extents[i] < 0) {
            return false;
        }
    }
    
    t = (tMin > 0) ? tMin : tMax;
    return t >= 0;
}