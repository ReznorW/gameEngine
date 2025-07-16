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
    void drawMainMenu(Window& window, Scene& scene, std::unique_ptr<Scene>& playScene, Camera& camera, Camera& playCamera, Mode& mode, bool& drawOBB);
    void drawSidebar(Scene& scene);
    void drawObjectTree(Object& obj, Scene& scene);
    void drawPlaytestUI(Scene& scene);

    // Popup Rendering
    void drawPopups(Scene& scene);
    void drawObjectPropertiesPopup(Scene& scene, Object* selected);
    void drawLoadScenePopup(Scene& scene);
    void drawSaveScenePopup(Scene& scene);
    void drawDeleteConfirmationPopup(Scene& scene);

private:
    // Popup bools
    bool openLoadScenePopup = false;
    bool openSaveScenePopup = false;
    bool openDeleteConfirmationPopup = false;

    // ImGui utils
    bool InputTextMultilineStdString(const char* label, std::string& str, const ImVec2& size, ImGuiInputTextFlags flags);
};
