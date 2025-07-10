#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "obb.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "resources.hpp"

// === Constructor ===
OBB::OBB(const glm::vec3& min, const glm::vec3& max) 
    : center((min + max) * 0.5f), extents(max - center), axes(glm::mat3(1.0f)) {}

// === Drawing ===
void drawOBB(const OBB& obb, const Camera& camera, Shader* debugShader, const glm::vec3& color) {
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

    debugShader->use();
    debugShader->setMat4("uViewProj", camera.getProjectionMatrix() * camera.getViewMatrix());
    debugShader->setMat4("uModel", model);
    debugShader->setVec3("uColor", color);

    glBindVertexArray(vao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}