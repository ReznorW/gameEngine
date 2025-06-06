#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include "scene.hpp"
#include "camera.hpp"

class Window {
public:
    Window(const std::string& title, bool fullscreen = false);
    ~Window();

    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;
    GLFWwindow* getGLFWwindow();
    int getWidth();
    int getHeight();

private:
    GLFWwindow* window;
    int width = 800;
    int height = 600;
    bool isFullscreen;
};

struct Context {
    Window* window;
    Camera* camera;
    Scene* scene;
};