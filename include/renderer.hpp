#pragma once

class Context;
class Camera;
class Object;
class Shader;
class Scene;

struct PointLight {
    int ID;
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float near;
    float far;
    unsigned int depthCubemap;
    unsigned int FBO;
};

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    int numOfCascades = 4;
    std::vector<float> shadowCascadeLevels;
    unsigned int depthMap;
    unsigned int FBO;
    unsigned int UBO;
};

class Renderer {
public:
    // Setting toggles
    bool drawDirectionalShadows = true;
    bool drawPointShadows = true;
    bool drawOBBs = false;

    // Constructors
    Renderer(const Camera& camera);
    Renderer(const Renderer& other);

    // Deconstructors
    ~Renderer();

    // Getters
    unsigned int getDirectionalShadowRes() const {return directionalShadowRes;}
    unsigned int getPointShadowRes() const {return pointShadowRes;}
    std::vector<PointLight>& getPointLights() {return pointLights;}
    PointLight& getPointLight(int ID);
    DirectionalLight& getDirectionalLight() {return directionalLight;}

    // Setters
    void setDirectionalShadowRes(unsigned int newRes) {directionalShadowRes = newRes;}
    void setPointShadowRes(unsigned int newRes) {pointShadowRes = newRes;}
    void setPointLight(PointLight newLight, int index) {pointLights[index] = newLight;}
    void setDirectionalLight(DirectionalLight newLight) {directionalLight = newLight;}

    // Point light functions
    int addPointLight(PointLight newLight);
    void removePointLight(int ID);

    // Rendering
    void renderDirectionalLight(Camera& camera, std::vector<Object*> objects);
    void renderPointLight(int index, std::vector<Object*> objects);
    void renderOBBs(Camera& camera, std::vector<Object*> objects);
    void renderScene(Context& context, Scene& scene, Camera& camera, bool inPlaytest);
    void reloadShaders() {initializeShaders();}

private:
    // ID tracker
    int nextPointLightID = 0;

    // Shadow resolutions
    unsigned int directionalShadowRes = 2048;
    unsigned int pointShadowRes = 1024;

    // Light sources
    std::vector<PointLight> pointLights;
    DirectionalLight directionalLight;

    // Shaders
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> directionalShader;
    std::shared_ptr<Shader> pointShader;
    std::shared_ptr<Shader> OBBShader;

    // Internal functions
    void initializeDirectionalLight(float far);
    void initializePointLight(int index);
    void initializeShaders();
};