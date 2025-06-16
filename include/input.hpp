#pragma once

#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "window.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "mode.hpp"

class Input {
public:
    // Input state
    static bool keys[512];
    static bool previousKeys[512];
    static bool mouseButtons[5];

    // Input processing
    static void processEditorInput(Window& windowStruct, Camera& camera, Camera& playCamera, Scene& scene, std::unique_ptr<Scene>& playScene, Mode& mode);
    static void processPlaytestInput(Window& windowStruct, Camera& camera, std::unique_ptr<Scene>& playScene, Mode& mode);
    static void processMouseMovement(Camera& camera, float& xoffset, float& yoffset, bool constrainPitch = true);
    static bool isKeyPressedOnce(int key);

    // GLFW callbacks
    static void mouse_button_callback(GLFWwindow* glfwWindow, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void char_callback(GLFWwindow* window, unsigned int c);

    // Mode changing
    static void modeChange(Mode newMode, GLFWwindow* window);
};

// Raycasting utils
glm::vec3 calculateRayFromMouse(double mouseX, double mouseY, int screenWidth, int screenHeight, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
bool RayIntersectsOBB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const OBB& obb, float& t);