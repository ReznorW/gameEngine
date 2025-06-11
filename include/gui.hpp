#pragma once

#include "window.hpp"
#include "camera.hpp"
#include "scene.hpp"

class Gui {
public:
    // Constructor
    Gui(Window& window);

    // Shutdown
    static void shutdown();

    // Frame lifecycle
    static void beginFrame();
    static void endFrame();

    // Input syncing
    void syncMouseFromGLFW(GLFWwindow* window);
    void syncKeyboardFromGLFW(GLFWwindow* window);

    // Rendering
    void drawMainMenu(Window& window, Scene& scene, Camera& camera);
    void drawSidebar(Scene& scene);
    void drawObjectProperties(Scene& scene, Object* selected);
    void drawLoadScenePopup(Scene& scene);
    void drawSaveScenePopup(Scene& scene);
};
