#pragma once

#include "window.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "mode.hpp"

class Gui {
public:
    // Constructor
    Gui(Window& window);

    // Shutdown
    static void shutdown();

    // Frame lifecycle
    static void beginFrame();
    static void endFrame();

    // Input syncing
    void syncMouseFromGLFW(GLFWwindow* window);
    void syncKeyboardFromGLFW(GLFWwindow* window);

    // Gui Rendering
    void drawMainMenu(Window& window, Scene& scene, std::unique_ptr<Scene>& playScene, Camera& camera, Camera& playCamera, Mode& mode, bool& drawOBB, Project& project);
    void drawSidebar(Scene& scene);
    void drawObjectTree(Object& obj, Scene& scene);
    void drawPlaytestUI(Scene& scene);
    void drawWelcomeScreen(Context& context);
    void drawScriptEditor(Context& context);
    void drawGizmos(Context& context);

    // Popup Rendering
    void drawPopups(Context& context);
    void drawObjectPropertiesPopup(Scene& scene, Object* selected, Project& project);
    void drawLoadScenePopup(Scene& scene, Mode& mode, Project& project);
    void drawSaveScenePopup(Scene& scene, const std::string& projectName);
    void drawDeleteConfirmationPopup(Scene& scene);
    void drawRenameScriptPopup(Context& context);
    void drawDocumentationPopup();
    void drawScenePropertiesPopup(Scene& scene);
    void drawLoadProjectPopup(Context& context);
    void drawNewProjectNamePopup(Context& context);

    // Gizmo Handling
    bool isGizmoVisible() {return gizmoVisible;}
    ImGuizmo::OPERATION getGizmoMode() {return currentGizmoOperation;}
    void setGizmoVisible(bool visible) {gizmoVisible = visible;}
    void setGizmoMode(ImGuizmo::OPERATION newOperation) {currentGizmoOperation = newOperation;}

private:
    // Popup bools
    bool openLoadScenePopup = false;
    bool openSaveScenePopup = false;
    bool openDeleteConfirmationPopup = false;
    bool openRenameScriptPopup = false;
    bool openHelpPopup = false;
    bool openScenePropertiesPopup = false;
    bool openLoadProjectPopup = false;
    bool openNewProjectNamePopup = false;

    // Popup vars
    std::shared_ptr<Script> scriptToRename = nullptr;
    std::string documentation;

    // Gizmo vars
    ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
    bool gizmoVisible = true;
};

// ImGui utils
bool InputTextMultilineStdString(const char* label, std::string& str, const ImVec2& size, ImGuiInputTextFlags flags);
int InputTextCallback(ImGuiInputTextCallbackData* data);
