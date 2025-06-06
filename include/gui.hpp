#pragma once

#include "window.hpp"
#include "camera.hpp"
#include "scene.hpp"

class Gui {
public:
    static void init(Window& window);
    static void beginFrame();
    static void endFrame();
    static void shutdown();
    void drawMainMenu(Window& window, Scene& scene, Camera& camera, Shader& shader);
    void drawSidebar(Scene& scene);
    void syncMouseFromGLFW(GLFWwindow* window);
    void syncKeyboardFromGLFW(GLFWwindow* window);
};
