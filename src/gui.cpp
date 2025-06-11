#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

#include "gui.hpp"

// === Constructor ===
Gui::Gui(Window& window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
    io.BackendFlags &= ~ImGuiBackendFlags_HasMouseCursors;

    ImGui_ImplGlfw_InitForOpenGL(window.getGLFWwindow(), false);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
}

// === Shutdown ===
void Gui::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// === Frame lifecycle ===
void Gui::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// === Input syncing ===
void Gui::syncMouseFromGLFW(GLFWwindow* window) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Get mouse position from GLFW
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    io.MousePos = ImVec2((float)mouseX, (float)mouseY);
    
    // Get mouse buttons from GLFW
    io.MouseDown[0] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    io.MouseDown[1] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    io.MouseDown[2] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
}

void Gui::syncKeyboardFromGLFW(GLFWwindow* window) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Synchronize modifier keys
    io.KeyCtrl = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || 
                 (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
    io.KeyShift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || 
                  (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    io.KeyAlt = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) || 
                (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
    io.KeySuper = (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) || 
                  (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS);
}

// === Rendering ===
void Gui::drawMainMenu(Window& window, Scene& scene, Camera& camera) {
    if (ImGui::BeginMainMenuBar()) {

        // File Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {
                scene.clear();
            }
            if (ImGui::MenuItem("Save Scene")) {
                // TODO: Implement scene saving
            }
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window.getGLFWwindow(), true);
            }
            ImGui::EndMenu();
        }

        // Edit Menu
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("New Object")) {
                std::string objName = "NewObj" + std::to_string(scene.getObjectCount());
                scene.addObject(objName, std::make_unique<Object>(objName, "cube", "default", "default"));
                scene.selectObject(objName);
            }
            if (ImGui::MenuItem("Undo")) {
                // TODO: Implement undo stack
            }
            if (ImGui::MenuItem("Redo")) {
                // TODO: Implement redo stack
            }
            ImGui::EndMenu();
        }

        // Selection Menu
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Deselect All")) {
                scene.clearSelection();
            }
            if (ImGui::MenuItem("Duplicate Selection")) {
                Object* selected = scene.getSelectedObject();
                if (selected) {
                    std::string newName = scene.duplicateObject(selected->name);
                    if (!newName.empty()) {
                        scene.selectObject(newName);
                    }
                }
            }
            ImGui::EndMenu();
        }

        // FPS counter
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100.0f);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::EndMainMenuBar();
    }

    // Show object properties if one is selected
    if (Object* selected = scene.getSelectedObject()) {
        drawObjectProperties(scene, selected);
    }
}

void Gui::drawSidebar(Scene& scene) {
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(200, ImGui::GetIO().DisplaySize.y - 20));
    ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    auto objects = scene.getObjects();
    for (Object* obj : objects) {
        bool isSelected = (scene.getSelectedObject() == obj);
        if (ImGui::Selectable(obj->name.c_str(), isSelected)) {
            scene.selectObject(obj->name);
        }
    }

    ImGui::End();
}

void Gui::drawObjectProperties(Scene& scene, Object* selected) {
    if (ImGui::Begin("Object Properties")) {
        // Editable Name
        char nameBuffer[128];
        std::strncpy(nameBuffer, selected->name.c_str(), sizeof(nameBuffer));
        nameBuffer[sizeof(nameBuffer) - 1] = '\0'; // Ensure null-termination

        ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            std::string newName(nameBuffer);
            if (!newName.empty() && newName != selected->name) {
                std::string finalName = scene.renameObject(selected->name, newName);
                selected->name = finalName;
            }
        }

        // Transform controls
        ImGui::DragFloat3("Position", glm::value_ptr(selected->transform.position), 0.1f);
        ImGui::DragFloat3("Rotation", glm::value_ptr(selected->transform.rotation), 0.1f);
        ImGui::DragFloat3("Scale",    glm::value_ptr(selected->transform.scale),    0.1f);
        selected->transform.markDirty();

        // Mesh selector
        std::string currentMesh = selected->mesh ? selected->mesh->getName() : "None";

        if (ImGui::BeginCombo("Mesh", currentMesh.c_str())) {
            auto meshes = scene.getMeshes();
            for (Mesh* mesh : meshes) {
                const std::string& meshName = mesh->getName();
                bool isSelected = (meshName == currentMesh);
                if (ImGui::Selectable(meshName.c_str(), isSelected)) {
                    selected->mesh = mesh;
                    selected->initializeOBB(mesh->getMinBounds(), mesh->getMaxBounds());
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Shader selector
        std::string currentShader = selected->shader ? selected->shader->getName() : "None";

        if (ImGui::BeginCombo("Shader", currentShader.c_str())) {
            auto shaderNames = scene.getShaderNames();
            for (const auto& shaderName : shaderNames) {
                bool isSelected = (shaderName == currentShader);
                if (ImGui::Selectable(shaderName.c_str(), isSelected)) {
                    selected->shader = scene.getShader(shaderName);
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Texture selector
        std::string currentTextureName = selected->texture ? selected->texture->getName() : "None";

        if (ImGui::BeginCombo("Texture", currentTextureName.c_str())) {
            auto textures = scene.getTextures();
            for (Texture* tex : textures) {
                const std::string& texName = tex->getName();
                bool isSelected = (selected->texture == tex);
                if (ImGui::Selectable(texName.c_str(), isSelected)) {
                    selected->texture = tex;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Texture Scale");
        float scale[2] = { selected->textureScale.x, selected->textureScale.y };
        if (ImGui::InputFloat("Scale X", &scale[0], 0.01f, 1.0f, "%.3f")) {
            selected->textureScale.x = scale[0];
        }
        if (ImGui::InputFloat("Scale Y", &scale[1], 0.01f, 1.0f, "%.3f")) {
            selected->textureScale.y = scale[1];
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Delete object
    if (ImGui::Button("Delete Object")) {
        ImGui::OpenPopup("Confirm Delete");
    }
    if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this object?");
        if (ImGui::Button("Yes")) {
            scene.deleteObject(selected->name);
            scene.clearSelection();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}
