#pragma once

#include <GLFW/glfw3.h>
#include <string>

#include "scene.hpp"
#include "camera.hpp"
#include "mode.hpp"
#include "project.hpp"

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

    // Setters
    void setSize(int w, int h);

private:
    // Window data
    GLFWwindow* window;
    int width = 1536;
    int height = 864;
    bool isFullscreen;
};

// Context defintion
struct Context {
    std::unique_ptr<Project> project;
    std::unique_ptr<Window> window;
    std::unique_ptr<Camera> sceneCamera;
    std::unique_ptr<Camera> playCamera;
    std::unique_ptr<Scene> editorScene;
    std::unique_ptr<Scene> playScene;
    Mode currentMode;
    Mode previousMode;

    Context() = default;
};