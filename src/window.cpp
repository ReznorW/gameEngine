#include <glad/glad.h>
#include <iostream>

#include "window.hpp"

// === Constructor ===
Window::Window(const std::string& title, bool fullscreen)
    : isFullscreen(fullscreen) {
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        window = nullptr;
        return;
    }

    // Set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Get monitor
    GLFWmonitor* monitor = nullptr;
    const GLFWvidmode* mode = nullptr;

    // Check if fullscreen
    if (fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        mode = glfwGetVideoMode(monitor);
        width = mode->width;
        height = mode->height;
    }

    // Create window
    window = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    // Bind OpenGL to current thread
    glfwMakeContextCurrent(window);

    // Resize viewport when screen resizes
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) {
        glViewport(0, 0, w, h);
        
        // Set new dims
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
        self->width = w;
        self->height = h;
    });

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        std::exit(-1);
    }
}

// === Deconstructor ===
Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

// === Window control ===
// Check if user closed window
bool Window::shouldClose() const { 
    return glfwWindowShouldClose(window); 
}

// Process all pending input events
void Window::pollEvents() const{ 
    glfwPollEvents(); 
}

// Swaps back and front buffers
void Window::swapBuffers() const{ 
    glfwSwapBuffers(window); 
}

// === Getters ===
GLFWwindow* Window::getGLFWwindow() { 
    return window; 
}

int Window::getWidth() {
    return width;
}

int Window::getHeight() {
    return height;
}
