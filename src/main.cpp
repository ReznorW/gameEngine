#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <glm/gtx/norm.hpp>
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
#include "mode.hpp"
#include "script.hpp"
#include "resources.hpp"

int main() {
    // === Context setup ===
    Context context;

    // === Window setup ===
    std::cout << "===Setting up window===" << std::endl;
    context.window = std::make_unique<Window>("Vertex Game Engine", false);
    Window& window = *context.window;
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1); // VSync

    // === Camera setup ===
    int lastWidth = window.getWidth();
    int lastHeight = window.getHeight();
    float aspect = static_cast<float>(lastWidth) / lastHeight;
    context.sceneCamera = std::make_unique<Camera>(aspect);
    context.playCamera = std::make_unique<Camera>(*context.sceneCamera);

    // === Gui setup ===
    std::cout << "===Setting up GUI===" << std::endl;
    Gui gui(window);

    // === Resource setup ===
    std::cout << "===Loading resources===" << std::endl;
    auto resources = std::make_shared<Resources>();

    // === Scene and objects ===
    std::cout << "===Initializing scene===" << std::endl;
    context.editorScene = std::make_unique<Scene>(resources.get());
    context.playScene = std::make_unique<Scene>(*context.editorScene);

    // === Initialize mode ===
    context.currentMode = Mode::WelcomeScreen;
    context.previousMode = Mode::WelcomeScreen;

    // === Input setup ===
    std::cout << "===Setting up input===" << std::endl;
    glfwSetWindowUserPointer(window.getGLFWwindow(), &context);
    glfwSetMouseButtonCallback(window.getGLFWwindow(), Input::mouse_button_callback);
    glfwSetCursorPosCallback(window.getGLFWwindow(), Input::cursor_position_callback);
    glfwSetKeyCallback(window.getGLFWwindow(), Input::key_callback);
    glfwSetCharCallback(window.getGLFWwindow(), Input::char_callback);
    glfwSetFramebufferSizeCallback(window.getGLFWwindow(), Input::framebuffer_size_callback);

    // === Timing setup ===
    const double timestep = 1.0 / 60.0;
    double accumulator = 0.0;
    double currentTime = glfwGetTime();

    // === Constants setup ===
    const float EPSILON = 1e-4f;
    const glm::vec3 GRAVITY = glm::vec3(0.0f, -5.0f, 0.0f);
    bool drawOBBs = false;

    // === Main loop ===
    std::cout << "===Rendering===" << std::endl;
    while (!window.shouldClose()) {
        // === Poll for events ===
        window.pollEvents();

        double newTime = glfwGetTime();
        float dt = newTime - currentTime;
        currentTime = newTime;
        accumulator += dt;

        // === Mode transition handling ===
        if (context.currentMode != context.previousMode) {
            Input::modeChange(context);
            context.previousMode = context.currentMode;
        }


        // Synchronize mouse before ImGui frame
        gui.syncMouseFromGLFW(window.getGLFWwindow());
        gui.syncKeyboardFromGLFW(window.getGLFWwindow());

        // === GUI begin ===
        gui.beginFrame();

        // === Process input ===
        while (accumulator >= timestep) {
            Input::processInput(context, dt);
            accumulator -= timestep;
        }

        // === Editor mode ===
        if (context.currentMode == Mode::SceneEditor) {
            Camera& camera = *context.sceneCamera;
            Scene& scene = *context.editorScene;

            // === Flush screen ===
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            // === OBB updating ===
            for (auto& obj : scene.getObjects()) {
                if (obj->transform.needsUpdate()) {
                    obj->updateOBB();
                }
            }

            scene.draw(camera, false, drawOBBs);

            // === Draw editor GUI ===
            gui.drawMainMenu(window, scene, context.playScene, camera, *context.playCamera, context.currentMode, drawOBBs);
            gui.drawSidebar(scene);
            gui.drawPopups(context);
        }

        // === Playtest mode ===
        else if (context.currentMode == Mode::Playtest) {
            Camera& camera = *context.playCamera;
            Scene& scene = *context.playScene;

            // === Flush screen ===
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            for (auto& obj : scene.getObjects()) {
                // Run scripts
                if (obj->script) {
                    obj->script->update(dt);
                }

                // Apply gravity
                if (obj->hasGravity) {
                    obj->transform.velocity += GRAVITY * dt;
                }

                // Apply velocity
                if (glm::length2(obj->transform.velocity) > 0.0f) {
                    if (glm::length2(obj->transform.velocity) > EPSILON) {
                        obj->transform.position += obj->transform.velocity * dt;
                        obj->transform.velocity *= 0.90f;
                    } else {
                        obj->transform.velocity = glm::vec3(0.0f);
                    }
                    obj->transform.markDirty();
                }

                // Resolve collisions
                if (obj->transform.needsUpdate()) {
                    obj->updateOBB();
                    if (obj->hasCollisions) {
                        for (auto& other : scene.getObjects()) {
                            if (other != obj && areIntersecting(*obj, *other)) {
                                resolveCollision(*obj, *other);
                            }
                        }
                    }
                }

                // Move camera with player
                if (obj->isPlayer) {
                    camera.position = obj->transform.position;
                    obj->transform.rotation.y = -camera.yaw;
                    obj->transform.markDirty();
                }
            }

            scene.draw(camera, true, drawOBBs);
            scene.processPendingDeletes();
            gui.drawPlaytestUI(scene);
        }

        else if (context.currentMode == Mode::WelcomeScreen) {
            // === Flush screen ===
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            gui.drawWelcomeScreen(context);
            gui.drawPopups(context);
        }

        // === GUI end ===
        gui.endFrame();

        // === Buffer Swap ===
        window.swapBuffers();

        // === Update camera aspect ratio ===
        int width = window.getWidth();
        int height = window.getHeight();
        if (width != lastWidth || height != lastHeight) {
            context.sceneCamera->setAspectRatio(static_cast<float>(width) / height);
            context.playCamera->setAspectRatio(static_cast<float>(width) / height);
            lastWidth = width;
            lastHeight = height;
        }

    }

    // === Cleanup ===
    gui.shutdown();

    return 0;
}
