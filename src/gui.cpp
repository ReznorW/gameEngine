#include <imgui.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui_markdown.h"
#include "ImGuizmo.h"

#include "gui.hpp"
#include "mode.hpp"
#include "object.hpp"
#include "mesh.hpp"
#include "script.hpp"
#include "project.hpp"

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
void Gui::drawMainMenu(Window& window, Scene& scene, std::unique_ptr<Scene>& playScene, Camera& camera, Camera& playCamera, Mode& mode, bool& drawOBB, Project& project) {
    if (ImGui::BeginMainMenuBar()) {
        // File Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Crtl+N")) {
                scene.clear();
            }
            if (ImGui::MenuItem("Open", "Crtl+O")) {
                openLoadScenePopup = true; 
            }
            if (ImGui::MenuItem("Save As", "Crtl+Shift+S")) {
                openSaveScenePopup = true;
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                const std::string& sceneName = scene.getName();
                if (!sceneName.empty()) {
                    scene.saveScene(sceneName, project.name);
                } else {
                    openSaveScenePopup = true;
                }
            }
            if (ImGui::MenuItem("Exit", "Ctrl+Q")) {
                glfwSetWindowShouldClose(window.getGLFWwindow(), true);
            }
            ImGui::EndMenu();
        }

        // Edit Menu
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("New Object", "C")) {
                std::string objName = "NewObj" + std::to_string(scene.getObjectCount());
                scene.addObject(objName, std::make_shared<Object>(objName, "cube", "default.jpg", "", project.resources));
                scene.selectObject(objName);
            }
            if (ImGui::MenuItem("Scene Properties")) {
                openScenePropertiesPopup = true;
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
            if (ImGui::MenuItem("Deselect", "Escape")) {
                scene.clearSelection();
            }
            if (ImGui::MenuItem("Duplicate Selection", "X")) {
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

        // View Menu
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Script Editor", "F2")) {
                mode = Mode::ScriptEditor;
            }
            ImGui::EndMenu();
        }

        // Run Menu
        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Playtest", "R")) {
                if (mode == Mode::SceneEditor) {
                    mode = Mode::Playtest;
                    playScene = std::make_unique<Scene>(scene);
                    playScene->clearSelection();
                    for (auto& obj : playScene->getObjects()) {
                        if (obj->isPlayer) {
                            playCamera.position = obj->transform.position;
                            playCamera.yaw = -obj->transform.rotation.y;
                            playCamera.pitch = obj->transform.rotation.x;
                            playCamera.updateCameraVectors();
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }

        // Settings Menu
        if (ImGui::BeginMenu("Settings")) {
            ImGui::MenuItem("Bounding Boxes", nullptr, &scene.renderer->drawOBBs);
            ImGui::MenuItem("Directional Shadows", nullptr, &scene.renderer->drawDirectionalShadows);
            ImGui::MenuItem("Point Shadows", nullptr, &scene.renderer->drawPointShadows);
            ImGui::EndMenu();
        }

        // FPS counter
        float menuWidth = ImGui::GetWindowWidth();
        float textWidth = ImGui::CalcTextSize("FPS: 000.0").x;
        ImGui::SetCursorPosX(menuWidth - textWidth - 20.0f);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::EndMainMenuBar();
    }
}

void Gui::drawSidebar(Scene& scene, Project& project) {
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(200, ImGui::GetIO().DisplaySize.y - 20));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoCollapse 
                           | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Objects", nullptr, flags);

    // Scene object hierarchy
    ImGui::Text("Scene Objects:");
    ImGui::BeginChild("SceneObjects", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.45f), true);
    for (auto& obj : scene.getObjects()) {
        if (obj->parent == nullptr) {
            drawObjectTree(*obj, scene);
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Project file browser
    ImGui::Text("Project Files:");
    static std::filesystem::path root = "projects/" + project.name;
    ImGui::BeginChild("ProjectFiles", ImVec2(0, 0), true);
    drawFileBrowser(root, project);
    ImGui::EndChild();

    ImGui::End();
}

void Gui::drawObjectTree(Object& obj, Scene& scene) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (scene.getSelectedObject() == &obj) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool nodeOpen = false;
    if (!obj.children.empty()) {
        nodeOpen = ImGui::TreeNodeEx(obj.name.c_str(), flags);
    } else {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::TreeNodeEx(obj.name.c_str(), flags);
    }

    if (ImGui::IsItemClicked()) {
        scene.selectObject(obj.name);
    }

    if (nodeOpen) {
        for (Object* child : obj.children) {
            drawObjectTree(*child, scene);
        }
        ImGui::TreePop();
    }
}

void Gui::cacheDirectory(const std::filesystem::path& dirPath, CachedEntry& outEntry) {
    namespace fs = std::filesystem;
    outEntry.name = dirPath.filename().string();
    if (outEntry.name.empty()) outEntry.name = dirPath.string();
    outEntry.isDirectory = true;
    outEntry.fullPath = dirPath.string();

    try {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            CachedEntry child;
            child.name = entry.path().filename().string();
            child.isDirectory = entry.is_directory();
            child.fullPath = entry.path().string();

            if (child.isDirectory) {
                cacheDirectory(entry.path(), child);
            }
            outEntry.children.push_back(std::move(child));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error caching directory " << dirPath << ": " << e.what() << std::endl;
    }
}

void Gui::drawCachedDirectory(const CachedEntry& entry) {
    ImGuiTreeNodeFlags flags = entry.isDirectory ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    if (entry.isDirectory) {
        bool open = ImGui::TreeNodeEx(entry.name.c_str(), flags);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            dropTargetPath = entry.fullPath;
        }

        if (open) {
            for (const auto& child : entry.children) {
                drawCachedDirectory(child);
            }
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("%s", entry.name.c_str());
    }
}

void Gui::drawFileBrowser(const std::filesystem::path& rootPath, Project& project) {
    if (directoryDirty) {
        directoryCacheRoot.clear();
        CachedEntry rootEntry;
        cacheDirectory(rootPath, rootEntry);
        directoryCacheRoot[rootPath.string()] = std::move(rootEntry);
        directoryDirty = false;
    }

    auto it = directoryCacheRoot.find(rootPath.string());
    if (it != directoryCacheRoot.end()) {
        drawCachedDirectory(it->second);
    }

    if (!droppedFiles.empty()) {
        std::filesystem::path target = dropTargetPath.empty() ? rootPath : dropTargetPath;
        std::string destDirectory = target.filename().string();

        for (const std::string& file : droppedFiles) {
            std::filesystem::path srcPath = file;
            std::filesystem::path destPath = target / srcPath.filename();

            std::string ext = srcPath.extension().string();

            if (destDirectory == "models" && ext == ".obj") {
                try {
                    std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);
                    project.resources->addMesh(std::make_shared<Mesh>(*loadObjFile(destPath.string())));
                } catch (const std::exception& e) {
                    std::cerr << "Failed to copy file: " << e.what() << "\n";
                }
            } else if (destDirectory == "textures" && (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")) {
                try {
                    std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);
                    project.resources->addTexture(std::make_shared<Texture>(srcPath.string()));
                } catch (const std::exception& e) {
                    std::cerr << "Failed to copy file: " << e.what() << "\n";
                }
            } else if (destDirectory == "scenes" && ext == ".scn") {
                try {
                    std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);
                } catch (const std::exception& e) {
                    std::cerr << "Failed to copy file: " << e.what() << "\n";
                }
            } else if (destDirectory == "scripts" && ext == ".lua") {
                try {
                    std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);
                    project.resources->addScript(std::make_shared<Script>(srcPath.string()));
                } catch (const std::exception& e) {
                    std::cerr << "Failed to copy file: " << e.what() << "\n";
                }
            }
        }

        droppedFiles.clear();
        directoryDirty = true;
    }
}

void Gui::drawPlaytestUI(Scene& scene) {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10, 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.35f);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoNav;

    ImGui::Begin("PlaytestLabel", nullptr, window_flags);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Playtest");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Objects: %d", scene.getObjectCount());
    ImGui::Text("Velocity: %f, %f, %f", scene.getPlayerObject()->transform.velocity.x, scene.getPlayerObject()->transform.velocity.y, scene.getPlayerObject()->transform.velocity.z);
    ImGui::End();
}

void Gui::drawWelcomeScreen(Context& context) {
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(displaySize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNav;

    ImGui::Begin("WelcomeScreen", nullptr, window_flags);

    ImVec2 cardSize(400, 300);
    ImVec2 cardPos((displaySize.x - cardSize.x) * 0.5f, (displaySize.y - cardSize.y) * 0.5f);
    ImGui::SetCursorPos(cardPos);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 24));

    ImGui::BeginChild("WelcomeCard", cardSize, true, ImGuiWindowFlags_NoScrollbar);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SetCursorPosX((cardSize.x - ImGui::CalcTextSize("Welcome to the Vertex Game Engine").x) * 0.5f);
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "Welcome to the Vertex Game Engine");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Dummy(ImVec2(0, ImGui::GetTextLineHeight()));
    
    ImGui::Spacing();
    ImGui::Spacing();

    float buttonWidth = 200;
    float buttonHeight = 40;
    float buttonX = (cardSize.x - buttonWidth) * 0.5f;

    ImGui::SetCursorPosX(buttonX);
    if (ImGui::Button("Create New Project", ImVec2(buttonWidth, buttonHeight))) {
        openNewProjectNamePopup = true;
    }

    ImGui::Spacing();

    ImGui::SetCursorPosX(buttonX);
    if (ImGui::Button("Load Project", ImVec2(buttonWidth, buttonHeight))) {
        openLoadProjectPopup = true;
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void Gui::drawScriptEditor(Context& context) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_MenuBar |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Script Editor", nullptr, window_flags);

    static std::shared_ptr<Script> currentScript = nullptr;
    static std::string currentScriptContent = "";
    static bool dirty = false;

    // Hotkey handling
    ImGuiIO& io = ImGui::GetIO();
    bool ctrlHeld = io.KeyCtrl;

    if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        if (currentScript && dirty) {
            currentScript->updateSource(currentScriptContent);
            currentScript->saveToFile();

            std::vector<Object*> matchingObjects;
            for (auto& object : context.editorScene->getObjects()) {
                if (object->script && object->script->getName() == currentScript->getName()) {
                    matchingObjects.push_back(object);
                }
            }

            for (auto& object : matchingObjects) {
                object->script = currentScript;
            }

            dirty = false;
        }
    }
    if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_N, false)) {
        currentScript = std::make_shared<Script>("new_script.lua");
        currentScriptContent = currentScript->getSource();
        currentScript->saveToFile();
        context.project->resources->addScript(currentScript);
    }
    if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_R, false)) {
        if (currentScript) {
            currentScriptContent = currentScript->getSource();
            dirty = false;
        }
    }
    if (ctrlHeld && ImGui::IsKeyPressed(ImGuiKey_H)) {
        if (documentation.empty()) {
            std::ifstream file("docs/Documentation.md");
            if (file) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                documentation = buffer.str();
            } else {
                documentation = "Failed to load Documentation.md.";
            }
        }
        openHelpPopup = true;
    }

    // Menu Bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Script", "Ctrl+N")) {
                currentScript = std::make_shared<Script>("new_script.lua");
                currentScriptContent = currentScript->getSource();
                currentScript->saveToFile();
                context.project->resources->addScript(currentScript);
            }
            if (ImGui::MenuItem("Save", "Ctrl+S", false, dirty && currentScript != nullptr)) {
                currentScript->updateSource(currentScriptContent);
                currentScript->saveToFile();

                std::vector<Object*> matchingObjects;
                for (auto& object : context.editorScene->getObjects()) {
                    if (object->script && object->script->getName() == currentScript->getName()) {
                        matchingObjects.push_back(object);
                    }
                }

                for (auto& object : matchingObjects) {
                    object->script = currentScript;
                }

                dirty = false;
            }
            if (ImGui::MenuItem("Reload", "Ctrl+R", false, currentScript != nullptr)) {
                currentScriptContent = currentScript->getSource();
                dirty = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Scene Editor", "F1")) {
                context.currentMode = Mode::SceneEditor;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("View Documentation", "Ctrl+H")) {
                if (documentation.empty()) {
                    std::ifstream file("docs/Documentation.md");
                    if (file) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        documentation = buffer.str();
                    } else {
                        documentation = "Failed to load Documentation.md.";
                    }
                }
                openHelpPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Layout
    ImGui::Columns(2, nullptr, true);
    ImGui::SetColumnWidth(0, 200);

    // Script list
    ImGui::BeginChild("Scripts", ImVec2(0, 0), true);
    auto scripts = context.project->resources->getScripts();
    for (const std::shared_ptr<Script>& script : scripts) {
        ImGui::PushID(script->getName().c_str());

        bool selected = (currentScript == script);
        if (ImGui::Selectable(script->getName().c_str(), selected)) {
            currentScript = script;
            currentScriptContent = currentScript->getSource();
            dirty = false;
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Rename")) {
                openRenameScriptPopup = true;
                scriptToRename = script;
            }
            if (ImGui::MenuItem("Delete")) {
                if (script) {
                    const std::string& filepath = "projects/" + context.project->name + "/scripts/" + script->getName() + ".lua";
                    if (!filepath.empty()) {
                        if (std::remove(filepath.c_str()) != 0) {
                            std::cerr << "Failed to delete script file: " << filepath << std::endl;
                        }
                    }

                    context.project->resources->deleteScript(script->getName());

                    if (currentScript == script) {
                        currentScript = nullptr;
                    }
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGui::EndChild();

    ImGui::NextColumn();

    // Text editor
    ImGui::BeginChild("Script Editor Area", ImVec2(0, 0), true);

    if (dirty) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Unsaved changes");
    }

    if (currentScript) {
        ImVec2 editorSize = ImVec2(-1, ImGui::GetContentRegionAvail().y * 0.7f);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize;
        if (InputTextMultilineStdString("##LuaText", currentScriptContent, editorSize, flags)) {
            dirty = true;
        }

        // Error log
        ImGui::Separator();
        ImGui::Text("Error Log:");
        ImGui::BeginChild("LuaErrorTerminal", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        const std::vector<std::string>& errorLog = currentScript->getErrorLog();
        if (!errorLog.empty()) {
            for (const auto& line : errorLog) {
                ImGui::TextUnformatted(line.c_str());
            }
        } else {
            ImGui::TextDisabled("No errors.");
        }

        ImGui::EndChild();
    } else {
        ImGui::Text("No script selected.");
    }

    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::End();
}

void Gui::drawGizmos(Context& context) {
    Scene& scene = *context.editorScene;
    Camera& camera = *context.sceneCamera;

    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    if (gizmoVisible && scene.getSelectedObject()) {
        Object* selected = scene.getSelectedObject();

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix();
        glm::mat4 model = selected->transform.getModelMatrix();

        glm::vec3 cameraPos = camera.getPosition();
        glm::vec3 objectPos = glm::vec3(model[3]);
        float distance = glm::distance(cameraPos, objectPos);
        float gizmoSize = glm::clamp(0.05f + 0.02f * std::log2(distance + 1.0f), 0.1f, 0.15f);
        ImGuizmo::SetGizmoSizeClipSpace(gizmoSize);

        ImGuizmo::Manipulate(
            glm::value_ptr(view),
            glm::value_ptr(projection),
            currentGizmoOperation,
            ImGuizmo::WORLD,
            glm::value_ptr(model)
        );

        if (ImGuizmo::IsUsing()) {
            selected->transform.setFromModelMatrix(model);
            selected->transform.markDirty();
        }
    }
}

// === Popup rendering ===
void Gui::drawPopups(Context& context) {
    Scene& scene = *context.editorScene;

    Object* selected = nullptr;
    if (context.editorScene) {
        selected = scene.getSelectedObject();
    }

    if (selected && context.currentMode == Mode::SceneEditor) {
        drawObjectPropertiesPopup(scene, scene.getSelectedObject(), *context.project);
    }

    if (openLoadScenePopup) {
        ImGui::OpenPopup("Load Scene Popup");
        openLoadScenePopup = false;
    }
    
    if (context.project) {
        drawLoadScenePopup(scene, context.currentMode, *context.project, *context.sceneCamera);
    }

    if (openSaveScenePopup) {
        ImGui::OpenPopup("Save Scene Popup");
        openSaveScenePopup = false;
    }
    drawSaveScenePopup(scene, context.project->name);

    if (openDeleteConfirmationPopup) {
        ImGui::OpenPopup("Confirm Delete");
        openDeleteConfirmationPopup = false;
    }
    drawDeleteConfirmationPopup(scene);

    if (openRenameScriptPopup) {
        ImGui::OpenPopup("Rename Script Popup");
        openRenameScriptPopup = false;
    }
    drawRenameScriptPopup(context);

    if(openHelpPopup && (context.currentMode == Mode::ScriptEditor)) {
        drawDocumentationPopup();
    }

    if (openScenePropertiesPopup && (context.currentMode == Mode::SceneEditor)) {
        drawScenePropertiesPopup(scene);
    }

    if (openLoadProjectPopup && (context.currentMode == Mode::WelcomeScreen)) {
        ImGui::OpenPopup("Load Project Popup");
        openLoadProjectPopup = false;
    }
    drawLoadProjectPopup(context);

    if (openNewProjectNamePopup && (context.currentMode == Mode::WelcomeScreen)) {
        ImGui::OpenPopup("New Project Name Popup");
        openNewProjectNamePopup = false;
    }
    drawNewProjectNamePopup(context);
}

void Gui::drawObjectPropertiesPopup(Scene& scene, Object* selected, Project& project) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 350, 20));
    ImGui::SetNextWindowSize(ImVec2(350, io.DisplaySize.y));

    if (!ImGui::Begin("Object Properties", nullptr, ImGuiWindowFlags_NoNav)) {
        ImGui::End();
        return;
    }

    // --- Basic info ---
    char nameBuffer[128];
    std::strncpy(nameBuffer, selected->name.c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';

    ImGui::SeparatorText("Name");
    ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        std::string newName(nameBuffer);
        if (!newName.empty() && newName != selected->name) {
            std::string finalName = scene.renameObject(selected->name, newName);
            selected->name = finalName;
        }
    }

    // --- Transform ---
    ImGui::SeparatorText("Transform");
    ImGui::DragFloat3("Position", glm::value_ptr(selected->transform.position), 0.1f);
    if (ImGui::DragFloat3("Rotation", glm::value_ptr(selected->transform.rotation), 0.1f)) {
        selected->transform.setRotation(selected->transform.rotation);
    }
    ImGui::DragFloat3("Scale",    glm::value_ptr(selected->transform.scale),    0.1f);
    selected->transform.markDirty();

    // --- Hierarchy ---
    ImGui::SeparatorText("Hierarchy");
    std::string currentParentName = selected->parent ? selected->parent->name : "None";
    if (ImGui::BeginCombo("Parent", currentParentName.c_str())) {
        if (ImGui::Selectable("None", selected->parent == nullptr)) {
            if (selected->parent) {
                auto& siblings = selected->parent->children;
                siblings.erase(std::remove(siblings.begin(), siblings.end(), selected), siblings.end());
                selected->parent = nullptr;
            }
        }

        for (Object* potentialParent : scene.getObjects()) {
            if (potentialParent == selected || selected->isDescendant(potentialParent))
                continue;

            bool isSelected = (selected->parent == potentialParent);
            if (ImGui::Selectable(potentialParent->name.c_str(), isSelected)) {
                selected->setParent(potentialParent);
            }
        }

        ImGui::EndCombo();
    }

    // --- Rendering ---
    ImGui::SeparatorText("Mesh");

    // Mesh
    std::string currentMesh = selected->mesh ? selected->mesh->getName() : "None";
    if (ImGui::BeginCombo("Mesh", currentMesh.c_str())) {
        for (const auto& mesh : project.resources->getMeshes()) {
            const std::string& meshName = mesh->getName();
            bool isSelected = (meshName == currentMesh);
            if (ImGui::Selectable(meshName.c_str(), isSelected)) {
                selected->mesh = mesh;
                selected->initializeOBB(mesh->getMinBounds(), mesh->getMaxBounds());
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // --- Material ---
    ImGui::SeparatorText("Material");
    ImGui::SliderFloat("Ambient",   &selected->material.ambient,   0.0f, 1.0f);
    ImGui::SliderFloat("Specular",  &selected->material.specular,  0.0f, 2.0f);
    ImGui::SliderFloat("Shininess", &selected->material.shininess, 1.0f, 128.0f);

    ImGui::SeparatorText("Lighting");
    if (selected->pointLightID >= 0) {
        PointLight& light = scene.renderer->getPointLight(selected->pointLightID);

        ImGui::Text("Light Settings");

        ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
        ImGui::SliderFloat("Intensity", &light.intensity, 0.0f, 1.0f);
        ImGui::DragFloat("Near", &light.near, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Far", &light.far, 0.1f, 0.1f, 100.0f);

        if (ImGui::Button("Remove Light")) {
            scene.renderer->removePointLight(selected->pointLightID);
            selected->pointLightID = -1;
        }
    } else {
        if (ImGui::Button("Add Light")) {
            PointLight light;
            light.position = selected->transform.position;
            light.color = glm::vec3(1.0f);
            light.intensity = 1.0f;
            light.near = 0.1f;
            light.far = 25.0f;
            int id = scene.renderer->addPointLight(light);
            selected->pointLightID = id;
        }
    }

    // --- Texture ---
    ImGui::SeparatorText("Texture");
    std::string currentTexture = selected->texture ? selected->texture->getName() : "None";
    if (ImGui::BeginCombo("Texture", currentTexture.c_str())) {
        for (const auto& tex : project.resources->getTextures()) {
            const std::string& texName = tex->getName();
            bool isSelected = (selected->texture == tex);

            ImGui::PushID(texName.c_str());
            ImGui::Image(tex->getID(), ImVec2(16, 16));
            ImGui::SameLine();

            if (ImGui::Selectable(texName.c_str(), isSelected)) {
                selected->texture = tex;
            }

            if (isSelected) ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    float scale[2] = {selected->textureScale.x, selected->textureScale.y};
    if (ImGui::InputFloat("Scale X", &scale[0], 0.01f, 1.0f, "%.3f")) {
        selected->textureScale.x = scale[0];
    }
    if (ImGui::InputFloat("Scale Y", &scale[1], 0.01f, 1.0f, "%.3f")) {
        selected->textureScale.y = scale[1];
    }

    // --- Physics ---
    ImGui::SeparatorText("Physics");

    if (ImGui::Checkbox("Player", &selected->isPlayer)) {
        if (selected->isPlayer) {
            for (auto& other : scene.getObjects()) {
                if (other != selected) {
                    other->isPlayer = false;
                    break;
                }
            }
        }
    }

    ImGui::Checkbox("Collisions", &selected->hasCollisions);
    ImGui::Checkbox("Moveable", &selected->isMoveable);
    ImGui::Checkbox("Gravity", &selected->hasGravity);

    // --- Script ---
    ImGui::SeparatorText("Script");

    std::string currentScript = selected->script ? selected->script->getName() : "None";
    if (ImGui::BeginCombo("Script", currentScript.c_str())) {
        bool noneSelected = (selected->script == nullptr);
        if (ImGui::Selectable("None", noneSelected)) {
            selected->script = nullptr;
        }
        if (noneSelected) ImGui::SetItemDefaultFocus();

        for (const auto& script : project.resources->getScripts()) {
            const std::string& scriptName = script->getName();
            bool isSelected = (selected->script == script);
            if (ImGui::Selectable(scriptName.c_str(), isSelected)) {
                selected->script = script;
                script->setOwner(selected);
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    // --- Deletion ---
    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Delete Object")) {
        openDeleteConfirmationPopup = true;
    }

    ImGui::End();
}

void Gui::drawLoadScenePopup(Scene& scene, Mode& mode, Project& project, Camera& camera) {
    static size_t selectedSceneIndex = 0;
    static bool initialized = false;
    std::vector<std::string> scenes = project.resources->getSceneNames(project.name);

    if (ImGui::BeginPopupModal("Load Scene Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select a scene to load:");

        if (!scenes.empty()) {
            if (!initialized) {
                std::string current = scene.getName();
                for (size_t i = 0; i < scenes.size(); ++i) {
                    if (scenes[i] == current) {
                        selectedSceneIndex = i;
                        break;
                    }
                }
                initialized = true;
            }

            if (selectedSceneIndex >= scenes.size()) {
                selectedSceneIndex = 0;
            }
            
            if (ImGui::BeginCombo("##SceneCombo", scenes[selectedSceneIndex].c_str())) {
                for (size_t i = 0; i < scenes.size(); ++i) {
                    bool isSelected = (selectedSceneIndex == i);
                    if (ImGui::Selectable(scenes[i].c_str(), isSelected)) {
                        selectedSceneIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Load")) {
                if (mode != Mode::SceneEditor) {
                    mode = Mode::SceneEditor;
                }
                scene.clear();
                scene.loadScene(scenes[selectedSceneIndex], project, camera);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::Text("No scenes available.");
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}

void Gui::drawSaveScenePopup(Scene& scene, const std::string& projectName) {
    static char saveFileName[128] = "";
    static bool popupJustClosed = false;

    if (ImGui::BeginPopupModal("Save Scene Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        popupJustClosed = false;
        ImGui::InputText("Filename", saveFileName, IM_ARRAYSIZE(saveFileName));

        if (ImGui::Button("Save")) {
            if (scene.saveScene(saveFileName, projectName)) {
                ImGui::CloseCurrentPopup();
                popupJustClosed = true;
            } else {
                // Handle save error
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            popupJustClosed = true;
        }

        ImGui::EndPopup();
    } else if (popupJustClosed) {
        saveFileName[0] = '\0';
        popupJustClosed = false;
    }
}

void Gui::drawDeleteConfirmationPopup(Scene& scene) {
    if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this object?");
        if (ImGui::Button("Yes")) {
            scene.deleteObject(scene.getSelectedObject()->name);
            scene.clearSelection();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Gui::drawRenameScriptPopup(Context& context) {
    static char nameBuffer[128] = "";
    static bool initialized = false;

    if (scriptToRename && ImGui::BeginPopupModal("Rename Script Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!initialized) {
            std::strncpy(nameBuffer, scriptToRename->getName().c_str(), sizeof(nameBuffer));
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';
            initialized = true;
        }

        ImGui::InputText("New Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));

        if (ImGui::Button("Rename")) {
            std::string newName(nameBuffer);
            if (!newName.empty() && newName != scriptToRename->getName()) {
                context.project->resources->renameScript(scriptToRename->getName(), newName);
            }
            scriptToRename = nullptr;
            initialized = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            scriptToRename = nullptr;
            initialized = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else {
        initialized = false;
    }
}

void Gui::drawDocumentationPopup() {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Documentation", &openHelpPopup)) {
        static ImGui::MarkdownConfig config;
        ImGui::BeginChild("DocScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Markdown(documentation.c_str(), documentation.size(), config);
        ImGui::EndChild();
    }
    ImGui::End();
}

void Gui::drawScenePropertiesPopup(Scene& scene) {
    ImGui::SetNextWindowSize(ImVec2(600, 800), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Scene Properties", &openScenePropertiesPopup)) {
        // Sky Color
        ImGui::Text("Environment");
        ImGui::ColorEdit4("Sky Color", glm::value_ptr(scene.skyColor));
        ImGui::DragFloat3("Sun Direction", glm::value_ptr(scene.renderer->getDirectionalLight().direction), 0.1f);
        ImGui::ColorEdit3("Sun Color", glm::value_ptr(scene.renderer->getDirectionalLight().color));
        ImGui::SliderFloat("Sun Intensity", &scene.renderer->getDirectionalLight().intensity, 0.0f, 1.0f);

        // Gravity
        ImGui::Text("Physics");
        ImGui::DragFloat3("Gravity", glm::value_ptr(scene.gravity), 0.1f);

        // Drag
        ImGui::SliderFloat("Global Drag", &scene.drag, 0.0f, 1.0f);

        // Player properties
        ImGui::Text("Player");
        ImGui::SliderFloat("Player Speed", &scene.playerSpeed, 0.1f, 10.0f);
        ImGui::SliderFloat("Jump Force", &scene.playerJump, 0.1f, 20.0f);

        ImGui::End();
    }
}

void Gui::drawLoadProjectPopup(Context& context) {
    enum class PopupStep {SelectProject, SelectScene};
    static PopupStep step = PopupStep::SelectProject;

    static std::vector<std::string> projectNames;
    static int selectedProjectIndex = -1;

    static std::vector<std::string> sceneFiles;
    static int selectedSceneIndex = -1;

    static std::string currentProjectName;
    static std::filesystem::path currentProjectPath;

    if (ImGui::BeginPopupModal("Load Project Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (step == PopupStep::SelectProject) {
            if (projectNames.empty()) {
                for (const auto& entry : std::filesystem::directory_iterator("projects")) {
                    if (entry.is_directory()) {
                        projectNames.push_back(entry.path().filename().string());
                    }
                }
            }

            ImGui::Text("Select a project to load:");
            ImGui::Spacing();

            if (ImGui::BeginListBox("##ProjectList", ImVec2(300, 200))) {
                for (size_t i = 0; i < projectNames.size(); ++i) {
                    bool isSelected = (static_cast<int>(i) == selectedProjectIndex);
                    if (ImGui::Selectable(projectNames[i].c_str(), isSelected)) {
                        selectedProjectIndex = i;
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::Spacing();

            float buttonWidth = 120;
            if (selectedProjectIndex >= 0) {
                if (ImGui::Button("Next", ImVec2(buttonWidth, 0))) {
                    currentProjectName = projectNames[selectedProjectIndex];
                    currentProjectPath = "projects/" + currentProjectName + "/scenes";

                    sceneFiles.clear();
                    for (const auto& entry : std::filesystem::directory_iterator(currentProjectPath)) {
                        if (entry.path().extension() == ".scn") {
                            sceneFiles.push_back(entry.path().filename().stem().string());
                        }
                    }

                    step = PopupStep::SelectScene;
                }
            } else {
                ImGui::BeginDisabled();
                if (ImGui::Button("Next", ImVec2(buttonWidth, 0))) {}
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                selectedProjectIndex = -1;
                projectNames.clear();
                ImGui::CloseCurrentPopup();
            }

        } else if (step == PopupStep::SelectScene) {
            ImGui::Text("Select a scene to load:");
            ImGui::Spacing();

            if (ImGui::BeginListBox("##SceneList", ImVec2(300, 200))) {
                for (size_t i = 0; i < sceneFiles.size(); ++i) {
                    bool isSelected = (static_cast<int>(i) == selectedSceneIndex);
                    if (ImGui::Selectable(sceneFiles[i].c_str(), isSelected)) {
                        selectedSceneIndex = i;
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::Spacing();
            float buttonWidth = 120;

            if (selectedSceneIndex >= 0) {
                if (ImGui::Button("Load Scene", ImVec2(buttonWidth, 0))) {
                    std::string selectedScene = sceneFiles[selectedSceneIndex];
                    std::filesystem::path fullScenePath = currentProjectPath / selectedScene;

                    // Resource and project setup
                    auto* resources = new Resources(currentProjectName);
                    context.project = std::make_unique<Project>(currentProjectName, resources);

                    // Camera setup
                    int w = context.window->getWidth();
                    int h = context.window->getHeight();
                    float aspect = static_cast<float>(w) / h;
                    context.sceneCamera = std::make_unique<Camera>(aspect);
                    context.playCamera = std::make_unique<Camera>(*context.sceneCamera);

                    // Load scene
                    context.editorScene = std::make_unique<Scene>();
                    context.editorScene->loadScene(selectedScene, *context.project, *context.sceneCamera);
                    context.playScene = std::make_unique<Scene>(*context.editorScene);

                    // Set mode
                    context.currentMode = Mode::SceneEditor;

                    // Cleanup
                    selectedProjectIndex = -1;
                    selectedSceneIndex = -1;
                    projectNames.clear();
                    sceneFiles.clear();
                    step = PopupStep::SelectProject;
                    ImGui::CloseCurrentPopup();
                }
            } else {
                ImGui::BeginDisabled();
                if (ImGui::Button("Load Scene", ImVec2(buttonWidth, 0))) {}
                ImGui::EndDisabled();
            }

            ImGui::SameLine();

            if (ImGui::Button("Back", ImVec2(buttonWidth, 0))) {
                sceneFiles.clear();
                selectedSceneIndex = -1;
                step = PopupStep::SelectProject;
            }
        }

        ImGui::EndPopup();
    } else {
        step = PopupStep::SelectProject;
        projectNames.clear();
        sceneFiles.clear();
        selectedProjectIndex = -1;
        selectedSceneIndex = -1;
    }
}

void Gui::drawNewProjectNamePopup(Context& context) {
    static char projectNameBuffer[128] = "UntitledProject";
    if (ImGui::BeginPopupModal("New Project Name Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter a name for your new project:");
        ImGui::InputText("##ProjectName", projectNameBuffer, IM_ARRAYSIZE(projectNameBuffer));

        ImGui::Spacing();

        if (ImGui::Button("Create", ImVec2(120, 0))) {
            std::string projectName = projectNameBuffer;

            std::filesystem::path projectPath = std::filesystem::path("projects") / projectName;

            std::error_code ec;
            if (!std::filesystem::exists(projectPath)) {
                if (!std::filesystem::create_directories(projectPath, ec)) {
                    std::cerr << "Failed to create project directories: " << ec.message() << std::endl;
                }
            }

            std::filesystem::create_directories(projectPath / "scenes", ec);
            std::filesystem::create_directories(projectPath / "scripts", ec);
            std::filesystem::create_directories(projectPath / "assets", ec);
            std::filesystem::create_directories(projectPath / "assets/models", ec);
            std::filesystem::create_directories(projectPath / "assets/textures", ec);

            Resources* newResources = new Resources(projectName);
            context.project = std::make_unique<Project>(projectName, newResources);
            context.editorScene = std::make_unique<Scene>();
            context.playScene = std::make_unique<Scene>(*context.editorScene);

            context.currentMode = Mode::SceneEditor;

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// === ImGui utils ===
bool InputTextMultilineStdString(const char* label, std::string& str, const ImVec2& size, ImGuiInputTextFlags flags) {
    flags |= ImGuiInputTextFlags_CallbackResize;

    if (str.empty()) {
        str = "";
    }

    bool changed = ImGui::InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags, InputTextCallback, (void*)&str);

    return changed;
}

int InputTextCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string* str = reinterpret_cast<std::string*>(data->UserData);
        IM_ASSERT(str != nullptr);
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    return 0;
}
