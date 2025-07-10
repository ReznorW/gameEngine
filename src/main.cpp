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
#include "mode.hpp"
#include "script.hpp"
#include "resources.hpp"

int main() {
    // === Context setup ===
    Context context;

    // === Window setup ===
    std::cout << "===Setting up window===" << std::endl;
    Window window("Game Engine", false);
    context.window = &window;
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1); // VSync

    // === Camera setup ===
    Camera editorCamera(static_cast<float>(window.getWidth()) / window.getHeight());
    Camera playCamera = editorCamera;
    context.camera = &editorCamera;

    // === Gui setup ===
    std::cout << "===Setting up GUI===" << std::endl;
    Gui gui(window);

    // === Resource setup ===
    std::cout << "===Loading resources===" << std::endl;
    auto resources = std::make_shared<Resources>();

    // === Scene and objects ===
    std::cout << "===Initializing scene===" << std::endl;
    Scene editorScene(resources.get());
    std::unique_ptr<Scene> playScene;
    context.scene = &editorScene;

    std::cout << "===Loading scene===" << std::endl;
    editorScene.loadScene("default");

    // === Initialize mode ===
    Mode mode = Mode::Editor;
    Mode prevMode = mode;
    context.mode = &mode;

    // === Input setup ===
    std::cout << "===Setting up input===" << std::endl;
    glfwSetWindowUserPointer(window.getGLFWwindow(), &context);
    glfwSetMouseButtonCallback(window.getGLFWwindow(), Input::mouse_button_callback);
    glfwSetCursorPosCallback(window.getGLFWwindow(), Input::cursor_position_callback);
    glfwSetKeyCallback(window.getGLFWwindow(), Input::key_callback);
    glfwSetCharCallback(window.getGLFWwindow(), Input::char_callback);

    // === Debug setup ===
    bool drawOBBs = false;

    // === Timing setup ===
    const double timestep = 1.0 / 60.0;
    double accumulator = 0.0;
    double currentTime = glfwGetTime();

    // Main render loop
    std::cout << "===Rendering===" << std::endl;
    while (!window.shouldClose()) {
        // === Poll for events ===
        window.pollEvents();

        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        accumulator += frameTime;

        // === Mode transition handling ===
        if (mode != prevMode) {
            Input::modeChange(mode, window.getGLFWwindow());
            if (prevMode == Mode::Editor) {
                for (auto& obj : playScene->getObjects()) {
                    if (obj->script) {
                        obj->script->setContext(&context);
                        obj->script->onStart();
                    }
                }
            }
            prevMode = mode;
        }

        // Synchronize mouse before ImGui frame
        gui.syncMouseFromGLFW(window.getGLFWwindow());
        gui.syncKeyboardFromGLFW(window.getGLFWwindow());

        // === GUI begin ===
        gui.beginFrame();

        // === Process input ===
        while (accumulator >= timestep) {
            if (mode == Mode::Editor) {
                Input::processEditorInput(window, editorCamera, playCamera, editorScene, playScene, mode);
            } else {
                Input::processPlaytestInput(window, playCamera, playScene, mode);
            }
            accumulator -= timestep;
        }

        // === Flush screen ===
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // === Editor mode ===
        if (mode == Mode::Editor) {
            context.camera = &editorCamera;
            context.scene = &editorScene;

            // === OBB updating ===
            for (auto& obj : editorScene.getObjects()) {
                if (obj->transform.needsUpdate()) {
                    obj->updateOBB();
                }
            }

            editorScene.draw(editorCamera, false, drawOBBs);

            // === Draw editor GUI ===
            gui.drawMainMenu(window, editorScene, playScene, editorCamera, playCamera, mode, drawOBBs);
            gui.drawSidebar(editorScene);
            gui.drawDeleteConfirmation(editorScene);
        }

        // === Playtest mode ===
        else if (mode == Mode::Playtest) {
            context.camera = &playCamera;
            context.scene = playScene.get();

            for (auto& obj : playScene->getObjects()) {
                if (obj->script) {
                    obj->script->update(frameTime);
                }

                if (obj->transform.needsUpdate()) {
                    obj->updateOBB();
                }

                if (obj->isPlayer) {
                    obj->transform.position = playCamera.position;
                    obj->transform.rotation.y = -playCamera.yaw;
                    obj->transform.markDirty();
                }
            }

            playScene->draw(playCamera, true, drawOBBs);

            playScene->processPendingDeletes();

            gui.drawPlaytestUI();
        }

        // === GUI end ===
        gui.endFrame();

        // === Buffer Swap and Events ===
        window.swapBuffers();

        // === Update camera aspect ratio ===
        editorCamera.setAspectRatio(static_cast<float>(window.getWidth()) / window.getHeight());
        playCamera.setAspectRatio(static_cast<float>(window.getWidth()) / window.getHeight());
    }

    // === Cleanup ===
    gui.shutdown();

    return 0;
}
