#pragma once

#include <GLFW/glfw3.h>
#include <string>

#include "scene.hpp"
#include "camera.hpp"
#include "mode.hpp"

// Window definition
class Window {
public:
    // Constructor
    Window(const std::string& title, bool fullscreen = false);

    // Deconstructor
    ~Window();

    // Window control
    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;

    // Getters
    GLFWwindow* getGLFWwindow();
    int getWidth();
    int getHeight();

private:
    // Window data
    GLFWwindow* window;
    int width = 1536;
    int height = 864;
    bool isFullscreen;
};

// Context defintion
struct Context {
    Window* window;
    Camera* camera;
    Scene* scene;
    Mode* mode;
};