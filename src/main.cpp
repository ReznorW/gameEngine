#include <glad/glad.h>
#include <iostream>
#include <chrono>
#include <memory>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "window.hpp"
#include "shader.hpp"
#include "input.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "object.hpp"
#include "scene.hpp"
#include "gui.hpp"

int main() {
    // === Context setup ===
    Context context;

    // === Window setup ===
    std::cout << "===Setting up window===" << std::endl;
    Window window("Game Engine", true);
    context.window = &window;
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1); // VSync

    // === Shader setup ===
    Shader shader("assets\\shaders\\default\\vertex.glsl", "assets\\shaders\\default\\fragment.glsl", "default");

    // === Camera setup ===
    Camera camera {
        glm::vec3(0.0f, 0.0f, 3.0f),  // position
        glm::vec3(0.0f, 0.0f, -1.0f), // front
        glm::vec3(1.0f, 0.0f, 0.0f),  // right
        glm::vec3(0.0f, 1.0f, 0.0f),  // up
        glm::vec3(0.0f, 1.0f, 0.0f),  // worldUp
        45.0f,                        // FOV
        static_cast<float>(window.getWidth()) / window.getHeight(), // aspect
        0.1f, 100.0f,                // near, far
        -90.0f, 0.0f, 0.0f           // yaw, pitch, roll
    };
    context.camera = &camera;
    camera.updateCameraVectors();

    // === Gui setup ===
    std::cout << "===Setting up GUI===" << std::endl;
    Gui gui;
    gui.init(window);

    // === Scene and objects ===
    std::cout << "===Initialize scene===" << std::endl;
    Scene scene;
    scene.init();
    context.scene = &scene;
    scene.addObject("cube", std::make_unique<Object>("cube", "cube", "default", &shader));
    scene.addObject("ground", std::make_unique<Object>("ground", "plane", "grass", &shader));

    if (Object* ground = scene.getObject("ground")) {
        ground->transform.position.y = -2.0f;
        ground->transform.scale = glm::vec3(100.0f);
    }

    // === Input setup ===
    std::cout << "===Setting up input===" << std::endl;
    glfwSetWindowUserPointer(window.getGLFWwindow(), &context);
    glfwSetMouseButtonCallback(window.getGLFWwindow(), Input::mouse_button_callback);
    glfwSetCursorPosCallback(window.getGLFWwindow(), Input::cursor_position_callback);
    glfwSetKeyCallback(window.getGLFWwindow(), Input::key_callback);
    glfwSetCharCallback(window.getGLFWwindow(), Input::char_callback);

    // === Timing setup ===
    const double timestep = 1.0 / 60.0;
    double accumulator = 0.0;
    double currentTime = glfwGetTime();
    double fpsTime = currentTime;
    int frameCount = 0;

    // Main render loop
    std::cout << "===Rendering===" << std::endl;
    while (!window.shouldClose()) {
        // === Poll for events ===
        window.pollEvents();

        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        accumulator += frameTime;

        // === FPS Counter ===
        frameCount++;
        if (newTime - fpsTime >= 1.0) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            fpsTime = newTime;
        }

        // Synchronize mouse before ImGui frame
        gui.syncMouseFromGLFW(window.getGLFWwindow());
        gui.syncKeyboardFromGLFW(window.getGLFWwindow());

        // === GUI begin ===
        gui.beginFrame();

        // === Process input ===
        while (accumulator >= timestep) {
            Input::processInput(window, camera, scene, shader);
            accumulator -= timestep;
        }

        // === Render ===
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (Object* cube = scene.getObject("cube")) {
            cube->transform.rotation.x = newTime * 15.0f;
            cube->transform.rotation.y = newTime * 20.0f;
            cube->transform.rotation.z = newTime * 5.0f;
            cube->transform.markDirty();
        }

        for (auto& obj : scene.getObjects()) {
            if (obj->transform.needsUpdate()) {
                obj->updateOBB();
            }
        }

        scene.draw(camera);

        // === Draw GUI ===
        gui.drawMainMenu(window, scene, camera, shader);
        gui.drawSidebar(scene);

        gui.endFrame();

        // === Buffer Swap and Events ===
        window.swapBuffers();

        // === Update camera aspect ratio ===
        camera.setAspectRatio(static_cast<float>(window.getWidth()) / window.getHeight());
    }

    // === Cleanup ===
    gui.shutdown();

    return 0;
}
