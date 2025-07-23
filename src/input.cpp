#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "input.hpp"
#include "camera.hpp"
#include "window.hpp"
#include "scene.hpp"
#include "mode.hpp"

// === Globals ===
bool mouseLookActive = false;
bool firstMouse = true; 
float lastX = 0.0f;
float lastY = 0.0f;

float movementSpeed = 0.1f;
float lookSpeed = 0.1f;

bool Input::keys[512] = {false};
bool Input::previousKeys[512] = {false};
bool Input::mouseButtons[5] = {false};

// === Input processing ===
void Input::processInput(Context& context, float dt) {
    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureKeyboard) {return;}

    switch (context.currentMode) {
        case Mode::SceneEditor:
            handleSceneEditorInput(*context.window, *context.sceneCamera, *context.editorScene, context.playScene, context.currentMode);
            break;

        case Mode::Playtest:
            handlePlaytestInput(*context.window, *context.playCamera, context.playScene, context.currentMode, dt);
            break;

        case Mode::ScriptEditor:
            handleScriptEditorInput(context.currentMode);
            break;

        case Mode::ModelEditor:
            // TODO: handleModelEditorInput()
            break;

        case Mode::WelcomeScreen:
            break;
    }

    std::memcpy(previousKeys, keys, sizeof(keys));
}

void Input::handleSceneEditorInput(Window& window, Camera& camera, Scene& scene, std::unique_ptr<Scene>& playScene, Mode& mode) {
    // --- Movement controls ---
    float currentSpeed = movementSpeed;

    // Speed boost (Left Ctrl)
    if (keys[GLFW_KEY_LEFT_CONTROL]) {
        currentSpeed *= 2.0f;
    }

    // Move forward (W)
    if (keys[GLFW_KEY_W]) {
        camera.move(glm::normalize(glm::vec3(camera.getFront().x, 0.0f, camera.getFront().z)), currentSpeed);
    }

    // Move backward (S)
    if (keys[GLFW_KEY_S] && !keys[GLFW_KEY_LEFT_CONTROL]) {
        camera.move(-glm::normalize(glm::vec3(camera.getFront().x, 0.0f, camera.getFront().z)), currentSpeed);
    }

    // Move left (A)
    if (keys[GLFW_KEY_A]) {
        camera.move(-camera.getRight(), currentSpeed);
    }

    // Move right (D)
    if (keys[GLFW_KEY_D]) {
        camera.move(camera.getRight(), currentSpeed);
    }

    // Move up (Space)
    if (keys[GLFW_KEY_SPACE]) {
        camera.moveVert(camera.getWorldUp(), currentSpeed);
    }

    // Move down (Left Shift)
    if (keys[GLFW_KEY_LEFT_SHIFT] && !keys[GLFW_KEY_LEFT_CONTROL]) {
        camera.moveVert(-camera.getWorldUp(), currentSpeed);
    }

    // --- Editor actions ---
    // Clear selection (Esc)
    if (isKeyPressedOnce(GLFW_KEY_ESCAPE)) {
        scene.clearSelection();
    }

    // Exit engine (Ctrl + Q)
    if (isKeyPressedOnce(GLFW_KEY_Q) && (keys[GLFW_KEY_LEFT_CONTROL] || keys[GLFW_KEY_RIGHT_CONTROL])) {
        glfwSetWindowShouldClose(window.getGLFWwindow(), true);
    }

    // Create new object (C)
    if (isKeyPressedOnce(GLFW_KEY_C)) {
        std::string objName = "NewObj" + std::to_string(scene.getObjectCount());
        scene.addObject(objName, std::make_unique<Object>(objName, "cube", "default.jpg", "default", "", scene.getResources()));
        scene.selectObject(objName);
    }

    // Delete selected object (Delete)
    if (isKeyPressedOnce(GLFW_KEY_DELETE) && scene.getSelectedObject()) {
        ImGui::OpenPopup("Confirm Delete");
    }

    // Duplicate selected object (X)
    if (isKeyPressedOnce(GLFW_KEY_X)) {
        if (Object* selected = scene.getSelectedObject()) {
            std::string newName = scene.duplicateObject(selected->name);
            if (!newName.empty()) scene.selectObject(newName);
        }
    }

    // Enter playtest (R)
    if (isKeyPressedOnce(GLFW_KEY_R)) {
        mode = Mode::Playtest;
        playScene = std::make_unique<Scene>(scene);
        playScene->clearSelection();
        for (auto& obj : playScene->getObjects()) {
            if (obj->isPlayer) {
                camera.position = obj->transform.position;
                camera.yaw = -obj->transform.rotation.y;
                camera.pitch = obj->transform.rotation.x;
                camera.updateCameraVectors();
            }
        }
    }

    // Save scene (Ctrl + S)
    if (isKeyPressedOnce(GLFW_KEY_S) && (keys[GLFW_KEY_LEFT_CONTROL] || keys[GLFW_KEY_RIGHT_CONTROL])) {
        const std::string& sceneName = scene.getName();
        if (!sceneName.empty()) {
            scene.saveScene(sceneName);
        } else {
            ImGui::OpenPopup("Save Scene Popup");
        }
    }

    // Save scene as (Ctrl + Shift + S)
    if (isKeyPressedOnce(GLFW_KEY_S) && (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT_SHIFT]) && (keys[GLFW_KEY_LEFT_CONTROL] || keys[GLFW_KEY_RIGHT_CONTROL])) {
        ImGui::OpenPopup("Save Scene Popup");
    }

    // Open scene (Ctrl + O)
    if (isKeyPressedOnce(GLFW_KEY_O) && (keys[GLFW_KEY_LEFT_CONTROL] || keys[GLFW_KEY_RIGHT_CONTROL])) {
        ImGui::OpenPopup("Load Scene Popup");
    }

    // New scene (Ctrl + N)
    if (isKeyPressedOnce(GLFW_KEY_N) && (keys[GLFW_KEY_LEFT_CONTROL] || keys[GLFW_KEY_RIGHT_CONTROL])) {
        scene.clear();
    }

    // Switch to script editor (F2)
    if (keys[GLFW_KEY_F2]) {
        mode = Mode::ScriptEditor;
    }

    // Increase FOV (F9)
    if (keys[GLFW_KEY_F9] && camera.getFOV() < 135) {
        camera.setFOV(camera.getFOV() + lookSpeed);
    }

    // Decrease FOV (F10)
    if (keys[GLFW_KEY_F10] && camera.getFOV() > 20) {
        camera.setFOV(camera.getFOV() - lookSpeed);
    }
}

void Input::handlePlaytestInput(Window& window, Camera& camera, std::unique_ptr<Scene>& playScene, Mode& mode, float dt) {
    // --- Movement controls ---
    float currentSpeed = playScene->playerSpeed;

    // Speed boost (Left Shift)
    if (keys[GLFW_KEY_LEFT_SHIFT]) {
        currentSpeed *= 2.0f;
    }

    if (Object* player = playScene->getPlayerObject()) {
        glm::vec3 forward = glm::normalize(glm::vec3(camera.getFront().x, 0.0f, camera.getFront().z));
        glm::vec3 right   = glm::normalize(glm::vec3(camera.getRight().x, 0.0f, camera.getRight().z));
        glm::vec3 move(0.0f);

        // Move forward (W)
        if (keys[GLFW_KEY_W]) {
            move += forward;
        }

        // Move backward (S)
        if (keys[GLFW_KEY_S]) {
            move -= forward;
        }

        // Move left (A)
        if (keys[GLFW_KEY_A]) {
            move -= right;
        }

        // Move right (D)
        if (keys[GLFW_KEY_D]) {
            move += right;
        }

        // Move up (Space)
        if (isKeyPressedOnce(GLFW_KEY_SPACE) && player->isGrounded) {
            player->transform.velocity.y += playScene->playerJump;
            player->transform.markDirty();
            player->isGrounded = false;
        }

        // Apply movement
        if (glm::length(move) > 0.0f) {
            move = glm::normalize(move) * currentSpeed;
            player->transform.velocity += move;
            player->transform.markDirty();
        }
    }

    // --- Playtest actions ---
    // Exit playtest (Esc)
    if (keys[GLFW_KEY_ESCAPE]) {
        mode = Mode::SceneEditor;
        playScene.reset();
        return;
    }

    // Increase FOV (F1)
    if (keys[GLFW_KEY_F1] && camera.getFOV() < 135) {
        camera.setFOV(camera.getFOV() + lookSpeed);
    }

    // Decrease FOV (F2)
    if (keys[GLFW_KEY_F2] && camera.getFOV() > 20) {
        camera.setFOV(camera.getFOV() - lookSpeed);
    }
}

void Input::handleScriptEditorInput(Mode& mode) {
    // Switch to scene editor (F1)
    if (keys[GLFW_KEY_F1]) {
        mode = Mode::SceneEditor;
    }
}

void Input::processMouseMovement(Camera& camera, float& xoffset, float& yoffset, bool constrainPitch) {
    camera.setYaw(camera.getYaw() + xoffset);

    if (constrainPitch) {
        float pitch = camera.getPitch() + yoffset;
        pitch = std::clamp(pitch, -89.0f, 89.0f);
        camera.setPitch(pitch);
    }

    camera.updateCameraVectors();
}

bool Input::isKeyPressedOnce(int key) {
    return keys[key] && !previousKeys[key];
}

// === GLFW callbacks ===
void Input::mouse_button_callback(GLFWwindow* glfwWindow, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(glfwWindow, button, action, mods);

    if (button >= 0 && button < 5) {
        mouseButtons[button] = (action == GLFW_PRESS);
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }
    
    // Pass in context
    Context* context = static_cast<Context*>(glfwGetWindowUserPointer(glfwWindow));
    if (!context || !context->window) {
         return;
    }

    Window& window = *context->window;
    Mode mode = context->currentMode;

    // Playtest inputs
    if (mode == Mode::Playtest) {
        if (!mouseLookActive) {
            mouseLookActive = true;
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            int width, height;
            glfwGetWindowSize(glfwWindow, &width, &height);

            double centerX = width / 2.0;
            double centerY = height / 2.0;
            glfwSetCursorPos(glfwWindow, centerX, centerY);

            lastX = static_cast<float>(centerX);
            lastY = static_cast<float>(centerY);

            firstMouse = true;
        }
        return;
    }

    // Editor inputs
    if (mode == Mode::SceneEditor) {
        // Right Mouse Button controls
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                mouseLookActive = true;
                glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                int width, height;
                glfwGetWindowSize(glfwWindow, &width, &height);

                double centerX = width / 2.0;
                double centerY = height / 2.0;
                glfwSetCursorPos(glfwWindow, centerX, centerY);

                lastX = static_cast<float>(centerX);
                lastY = static_cast<float>(centerY);

                firstMouse = true;
            } else if (action == GLFW_RELEASE) {
                mouseLookActive = false;
                glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        // Left Mouse Button controls
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                if (!context->sceneCamera || !context->editorScene) return;

                Camera& camera = *context->sceneCamera;
                Scene& scene = *context->editorScene;

                double mouseX, mouseY;
                glfwGetCursorPos(window.getGLFWwindow(), &mouseX, &mouseY);

                int width, height;
                glfwGetWindowSize(window.getGLFWwindow(), &width, &height);

                glm::vec3 ray = calculateRayFromMouse(mouseX, mouseY, width, height, camera.getProjectionMatrix(), camera.getViewMatrix());

                scene.clearSelection();
                
                float closestT = std::numeric_limits<float>::max();
                Object* selected = nullptr;

                for (auto& obj : scene.getObjects()) {
                    float t;
                    if (RayIntersectsOBB(camera.getPosition(), ray, obj->obb, t) && t < closestT) {
                        closestT = t;
                        selected = obj;
                    }
                }

                if (selected) {
                    scene.selectObject(selected->name);
                }
            }
        }
    }
}

void Input::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)xpos, (float)ypos);
    
    if (!mouseLookActive) {return;}

    // Pass in context
    Context* context = static_cast<Context*>(glfwGetWindowUserPointer(window));
    if (!context) {return;}

    Camera* camera = nullptr;

    if (context->currentMode == Mode::SceneEditor && context->sceneCamera) {
        camera = context->sceneCamera.get();
    } else if (context->currentMode == Mode::Playtest && context->playCamera) {
        camera = context->playCamera.get();
    }
    
    if (!camera) {return;}

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // inverted Y

    lastX = xpos;
    lastY = ypos;

    float sensitivity = lookSpeed;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    Input::processMouseMovement(*camera, xoffset, yoffset);
}

void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    
    if (key >= 0 && key < 512) {
        keys[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

void Input::char_callback(GLFWwindow* window, unsigned int c) {
    ImGui_ImplGlfw_CharCallback(window, c);
}

void Input::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);

    Context* context = static_cast<Context*>(glfwGetWindowUserPointer(window));
    if (!context) return;

    context->window->setSize(width, height);

    if (context->sceneCamera)
        context->sceneCamera->setAspectRatio(static_cast<float>(width) / height);
    if (context->playCamera)
        context->playCamera->setAspectRatio(static_cast<float>(width) / height);
}

// === Raycasting utils ===
glm::vec3 calculateRayFromMouse(double mouseX, double mouseY, int screenWidth, int screenHeight, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
    // Convert mouse position to Normalized Device Coordinates (NDC)
    float ndcX = (2.0f * static_cast<float>(mouseX)) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * static_cast<float>(mouseY)) / screenHeight;

    // Ray in clip space (pointing forward)
    glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);

    // Convert to eye space
    glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Convert to world space
    glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(viewMatrix) * rayEye));
    return rayWorld;
}

bool RayIntersectsOBB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const OBB& obb, float& t) {
    float tMin = -FLT_MAX;
    float tMax = FLT_MAX;

    glm::vec3 delta = obb.center - rayOrigin;
    
    for (int i = 0; i < 3; i++) {
        glm::vec3 axis = obb.axes[i];
        float e = glm::dot(axis, delta);
        float f = glm::dot(axis, rayDir);
        
        if (std::abs(f) > 0.001f) {
            float t1 = (e + obb.extents[i]) / f;
            float t2 = (e - obb.extents[i]) / f;
            
            if (t1 > t2) {
                std::swap(t1, t2);
            }

            tMin = glm::max(tMin, t1);
            tMax = glm::min(tMax, t2);
            
            if (tMin > tMax || tMax < 0.0f) {
                return false;
            }
        } else if (-e - obb.extents[i] > 0.0f || -e + obb.extents[i] < 0.0f) {
            return false;
        }
    }
    
    t = (tMin > 0.0f) ? tMin : tMax;
    return t >= 0.0f;
}

// === Mode changing ===
void Input::modeChange(Context& context) {
    Mode newMode = context.currentMode;

    switch (newMode) {
        case Mode::SceneEditor:
            mouseLookActive = false;
            glfwSetInputMode(context.window->getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;

        case Mode::ScriptEditor:
            // Switch to script editor
            break;

        case Mode::ModelEditor:
            // Switch to model editor
            break;

        case Mode::WelcomeScreen:
            break;

        case Mode::Playtest:
            mouseLookActive = true;
            GLFWwindow* win = context.window->getGLFWwindow();
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            int width, height;
            glfwGetWindowSize(win, &width, &height);
            double centerX = width / 2.0;
            double centerY = height / 2.0;
            glfwSetCursorPos(win, centerX, centerY);

            lastX = static_cast<float>(centerX);
            lastY = static_cast<float>(centerY);
            firstMouse = true;

            if (context.playScene) {
                for (auto& obj : context.playScene->getObjects()) {
                    if (obj->script) {
                        obj->script->setContext(&context);
                        obj->script->setOwner(obj);
                        obj->script->onStart();
                    }
                }
            }

            break;
    }
}