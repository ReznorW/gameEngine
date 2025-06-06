#include <GLFW/glfw3.h>
#include "camera.hpp"
#include "window.hpp"
#include "scene.hpp"
#include "shader.hpp"

class Input {
public:
    static bool keys[512];
    static bool mouseButtons[5];

    static void processInput(Window& windowStruct, Camera& camera, Scene& scene, Shader& shader);
    static void processMouseMovement(Camera& camera, float& xoffset, float& yoffset, bool constrainPitch = true);
    static void mouse_button_callback(GLFWwindow* glfwWindow, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void char_callback(GLFWwindow* window, unsigned int c);
};

glm::vec3 calculateRayFromMouse(double mouseX, double mouseY, int screenWidth, int screenHeight, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
bool RayIntersectsOBB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const OBB& obb, float& t);