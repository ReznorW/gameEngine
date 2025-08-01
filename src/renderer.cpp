#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <ostream>

#include "renderer.hpp"
#include "window.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "shader.hpp"

// === Constructors ===
Renderer::Renderer(const Camera& camera) {
    initializeDirectionalLight(camera.far);
    initializeShaders();
}

Renderer::Renderer(const Renderer& other) :
    drawDirectionalShadows(other.drawDirectionalShadows),
    drawPointShadows(other.drawPointShadows),
    drawOBBs(other.drawOBBs),
    directionalShadowRes(other.directionalShadowRes),
    pointShadowRes(other.pointShadowRes),
    pointLights(other.pointLights),
    directionalLight(other.directionalLight),
    shader(other.shader),
    directionalShader(other.directionalShader),
    pointShader(other.pointShader),
    OBBShader(other.OBBShader) {}

// === Deconstructors ===
Renderer::~Renderer() {
    glDeleteTextures(1, &directionalLight.depthMap);
    glDeleteFramebuffers(1, &directionalLight.FBO);
    glDeleteBuffers(1, &directionalLight.UBO);
    for (const auto& light : pointLights) {
        glDeleteTextures(1, &light.depthCubemap);
        glDeleteFramebuffers(1, &light.FBO);
    }
}

// === Getters ===
PointLight& Renderer::getPointLight(int ID) {
    for (PointLight& light : pointLights) {
        if (light.ID == ID) {
            return light;
        }
    }
    throw std::runtime_error("PointLight with given index not found");
}

// === Point light functions ===
int Renderer::addPointLight(PointLight newLight) {
    int ID = nextPointLightID++;
    newLight.ID = ID;
    pointLights.push_back(newLight);

    initializePointLight(pointLights.size() - 1);

    return ID;
}

void Renderer::removePointLight(int ID) {
    int index = 0;
    for (PointLight& light : pointLights) {
        if (light.ID == ID) {
            glDeleteFramebuffers(1, &pointLights[index].FBO);
            glDeleteTextures(1, &pointLights[index].depthCubemap);
            pointLights.erase(pointLights.begin() + index);
            return;
        }
        index++;
    }
}

// === Rendering ===
void Renderer::renderDirectionalLight(Camera& camera, std::vector<Object*> objects) {
    // UBO setup
    const auto lightMatrices = camera.getLightSpaceMatrices(glm::normalize(-directionalLight.direction), directionalLight.shadowCascadeLevels);
    glBindBuffer(GL_UNIFORM_BUFFER, directionalLight.UBO);
    for (int i = 0; i < static_cast<int>(lightMatrices.size()); i++) {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Execute shader
    directionalShader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, directionalLight.FBO);
    glViewport(0, 0, directionalShadowRes, directionalShadowRes);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    for (const auto& obj : objects) {
        directionalShader->setMat4("model", obj->getWorldMatrix());
        obj->mesh->draw();
    }
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::renderPointLight(int index, std::vector<Object*> objects) {
    // Get point light
    PointLight& light = pointLights[index];

    // Create shadow matrices
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, light.near, light.far);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

    // Execute shader
    pointShader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, light.FBO);
    glViewport(0, 0, pointShadowRes, pointShadowRes);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (unsigned int i = 0; i < 6; i++) {
        pointShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    }
    pointShader->setFloat("far", light.far);
    pointShader->setVec3("lightPos", light.position);
    for (const auto& obj : objects) {
        pointShader->setMat4("model", obj->getWorldMatrix());
        obj->mesh->draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::renderOBBs(Camera& camera, std::vector<Object*> objects) {
    for (const auto& obj : objects) {
        OBB& obb = obj->obb;
        glm::vec3 e = obb.extents;
        glm::vec3 corners[8] = {
            {-e.x, -e.y, -e.z},
            { e.x, -e.y, -e.z},
            { e.x,  e.y, -e.z},
            {-e.x,  e.y, -e.z},
            {-e.x, -e.y,  e.z},
            { e.x, -e.y,  e.z},
            { e.x,  e.y,  e.z},
            {-e.x,  e.y,  e.z}
        };

        glm::mat4 model(1.0f);
        model[0] = glm::vec4(obb.axes[0], 0.0f);
        model[1] = glm::vec4(obb.axes[1], 0.0f);
        model[2] = glm::vec4(obb.axes[2], 0.0f);
        model[3] = glm::vec4(obb.center, 1.0f);

        unsigned int indices[24] = {
            0,1, 1,2, 2,3, 3,0,
            4,5, 5,6, 6,7, 7,4,
            0,4, 1,5, 2,6, 3,7
        };

        GLuint vao, vbo, ebo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        OBBShader->use();
        OBBShader->setMat4("uViewProj", camera.getProjectionMatrix() * camera.getViewMatrix());
        OBBShader->setMat4("uModel", model);
        OBBShader->setVec3("uColor", glm::vec3(1.0f, 0.0f, 0.0f)); // Red outline

        glBindVertexArray(vao);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }
}

void Renderer::renderScene(Context& context, Scene& scene, Camera& camera, bool inPlaytest) {
    // Execute shader
    shader->use();
    glViewport(0, 0, context.window->getWidth(), context.window->getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera uniforms
    shader->setMat4("projection", camera.getProjectionMatrix());
    shader->setMat4("view", camera.getViewMatrix());

    // Lighting uniforms
    shader->setBool("drawDirectionalShadows", drawDirectionalShadows);
    shader->setBool("drawPointShadows", drawPointShadows);
    shader->setInt("numPointLights", pointLights.size());
    for (int i = 0; i < static_cast<int>(pointLights.size()); i++) {
        shader->setVec3("pointLights[" + std::to_string(i) + "].position", pointLights[i].position);
        shader->setVec3("pointLights[" + std::to_string(i) + "].color", pointLights[i].color);
        shader->setFloat("pointLights[" + std::to_string(i) + "].intensity", pointLights[i].intensity);
        shader->setFloat("pointLights[" + std::to_string(i) + "].near", pointLights[i].near);
        shader->setFloat("pointLights[" + std::to_string(i) + "].far", pointLights[i].far);
    }
    shader->setVec3("directionalLight.direction", glm::normalize(directionalLight.direction));
    shader->setVec3("directionalLight.color", directionalLight.color);
    shader->setFloat("directionalLight.intensity", directionalLight.intensity);
    shader->setVec3("viewPos", camera.getPosition());
    shader->setFloat("cameraFar", camera.far);
    shader->setFloat("ambient", scene.ambient);
    shader->setInt("cascadeCount", directionalLight.numOfCascades);
    for (int i = 0; i < directionalLight.numOfCascades; i++) {
        shader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", directionalLight.shadowCascadeLevels[i]);
    }

    // Fog uniforms
    shader->setVec3("fogColor", glm::vec3(0.5f, 0.6f, 0.7f));
    shader->setFloat("fogStart", 50.0f);
    shader->setFloat("fogEnd", 100.0f);

    // Object uniforms
    Object* selected = scene.getSelectedObject();
    for (const auto& obj : scene.getObjects()) {
        bool isHighlighted = (obj == selected);

        if (!(inPlaytest && obj->isPlayer)) {
            obj->texture->bind(0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D_ARRAY, directionalLight.depthMap);
            for (int i = 0; i < static_cast<int>(pointLights.size()); i++) {
                glActiveTexture(GL_TEXTURE0 + 2 + i);
                glBindTexture(GL_TEXTURE_CUBE_MAP, pointLights[i].depthCubemap);
            }
            shader->setMat4("model", obj->getWorldMatrix());
            shader->setBool("isSelected", isHighlighted);
            shader->setFloat("specular", obj->material.specular);
            shader->setFloat("shininess", obj->material.shininess);
            shader->setVec2("textureScale", obj->textureScale);
        }

        obj->mesh->draw();
    }
}

// === Internal functions ===
void Renderer::initializeDirectionalLight(float far) {
    // Generate cascade levels
    directionalLight.shadowCascadeLevels = {far / 40.0f, far / 15.0f, far / 5.0f, far / 2.0f};

    // Initialize depth map array
    glGenTextures(1, &directionalLight.depthMap);
    glBindTexture(GL_TEXTURE_2D_ARRAY, directionalLight.depthMap);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, directionalShadowRes, directionalShadowRes, int(directionalLight.shadowCascadeLevels.size()) + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);	
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Initialize FBO
    glGenFramebuffers(1, &directionalLight.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, directionalLight.FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, directionalLight.depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize UBO
    glGenBuffers(1, &directionalLight.UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, directionalLight.UBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, directionalLight.UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::initializePointLight(int index) {
    // Get point light
    PointLight& light = pointLights[index];

    // Initialize cubemap
    glGenTextures(1, &light.depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, light.depthCubemap);
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, pointShadowRes, pointShadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Initialize FBO
    glGenFramebuffers(1, &light.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, light.FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light.depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initializeShaders() {
    // Load in shaders
    shader = std::make_shared<Shader>("default", false);
    directionalShader = std::make_shared<Shader>("directional", true);
    pointShader = std::make_shared<Shader>("point", true);
    OBBShader = std::make_shared<Shader>("OBB", false);

    // Initialize primary shader
    shader->use();
    shader->setInt("texture1", 0);
    shader->setInt("depthMap", 1);
    // Preallocate 16 cube maps for maximum 16 point lights
    for (int i = 0; i < 16; i++) {
        shader->setInt("depthCubemap[" + std::to_string(i) + "]", i + 2);
    }
}