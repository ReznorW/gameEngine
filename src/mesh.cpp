#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <limits>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "scene.hpp"
#include "mesh.hpp"

// === Constructors ===
Mesh::Mesh(const std::string& meshName, const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds)
    : name(meshName), vertices(verts), indices(inds) {
    setupMesh(vertices, indices);
    indexCount = indices.size();
}

Mesh::Mesh(const Mesh& other)
    : indexCount(other.indexCount), name(other.name), minBounds(other.minBounds), maxBounds(other.maxBounds), vertices(other.vertices), indices(other.indices) {
    setupMesh(vertices, indices);
}

// === Deconstructor ===
Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// === OBB handling ===
void Mesh::calculateBounds(const std::vector<Vertex>& vertices) {
    if (vertices.empty()) {
        minBounds = maxBounds = glm::vec3(0.0f);
        return;
    }

    // Initialize with first vertex
    minBounds = maxBounds = vertices[0].position;
        
    // Find min/max across all vertices
    for (const auto& vertex : vertices) {
        minBounds = glm::min(minBounds, vertex.position);
        maxBounds = glm::max(maxBounds, vertex.position);
    }
}

// === Rendering ===
void Mesh::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// === Internal setup ===
void Mesh::setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Texture coords attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

// === Loaders ===
Mesh* loadObjFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
        return nullptr;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::unordered_map<std::string, unsigned int> uniqueVertexMap;

    bool hasNormals = false;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        } else if (prefix == "vt") {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            texCoords.push_back(uv);
        } else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
            hasNormals = true;
        } else if (prefix == "f") {
            std::vector<unsigned int> faceIndices;
            std::string vertexStr;

            while (iss >> vertexStr) {
                if (uniqueVertexMap.count(vertexStr) == 0) {
                    size_t firstSlash = vertexStr.find('/');
                    size_t secondSlash = vertexStr.find('/', firstSlash + 1);

                    int posIdx = std::stoi(vertexStr.substr(0, firstSlash)) - 1;

                    int texIdx = -1;
                    if (secondSlash > firstSlash + 1) {
                        texIdx = std::stoi(vertexStr.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;
                    }

                    int normIdx = -1;
                    if (secondSlash + 1 < vertexStr.size()) {
                        normIdx = std::stoi(vertexStr.substr(secondSlash + 1)) - 1;
                    }

                    Vertex v;
                    v.position = positions[posIdx];
                    v.texCoords = (texIdx >= 0 && texIdx < (int)texCoords.size()) ? texCoords[texIdx] : glm::vec2(0.0f);
                    v.normal = (normIdx >= 0 && normIdx < (int)normals.size()) ? normals[normIdx] : glm::vec3(0.0f);

                    vertices.push_back(v);
                    unsigned int index = static_cast<unsigned int>(vertices.size() - 1);
                    uniqueVertexMap[vertexStr] = index;
                    faceIndices.push_back(index);
                } else {
                    faceIndices.push_back(uniqueVertexMap[vertexStr]);
                }
            }

            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                indices.push_back(faceIndices[0]);
                indices.push_back(faceIndices[i]);
                indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    if (!hasNormals) {
        for (size_t i = 0; i < indices.size(); i += 3) {
            Vertex& v0 = vertices[indices[i]];
            Vertex& v1 = vertices[indices[i + 1]];
            Vertex& v2 = vertices[indices[i + 2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            v0.normal = faceNormal;
            v1.normal = faceNormal;
            v2.normal = faceNormal;
        }
    }

    glm::vec3 minPos(FLT_MAX);
    glm::vec3 maxPos(-FLT_MAX);
    for (const auto& v : vertices) {
        minPos = glm::min(minPos, v.position);
        maxPos = glm::max(maxPos, v.position);
    }

    glm::vec3 center = (minPos + maxPos) * 0.5f;
    glm::vec3 size = maxPos - minPos;
    float maxExtent = glm::max(glm::max(size.x, size.y), size.z);
    float scale = (maxExtent > 0.0f) ? 1.0f / maxExtent : 1.0f;

    for (auto& v : vertices) {
        v.position = (v.position - center) * scale;
    }

    std::string name = filepath.substr(filepath.find_last_of("/\\") + 1);
    size_t dotPos = name.find_last_of('.');
    if (dotPos != std::string::npos) {
        name = name.substr(0, dotPos);
    }

    Mesh* mesh = new Mesh(name, vertices, indices);
    mesh->calculateBounds(vertices);
    return mesh;
}
