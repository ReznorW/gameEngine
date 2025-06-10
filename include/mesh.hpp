#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>

// Vertex defintion
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoords;
};

// Mesh definition
class Mesh {
public:
    // Constructor
    Mesh(const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    
    // Deconstructor
    ~Mesh();

    // Getters
    const glm::vec3& getMinBounds() const {return minBounds;}
    const glm::vec3& getMaxBounds() const {return maxBounds;}
    std::string getName() const {return name;}

    // OBB handling
    void calculateBounds(const std::vector<Vertex>& vertices);

    // Rendering
    void draw() const;

private:
    // OpenGL buffers
    unsigned int VAO, VBO, EBO;
    size_t indexCount;

    // Mesh data
    std::string name;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    // Internal setup
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

// Loaders
Mesh* loadVertFile(const std::string& filepath);
unsigned int loadTexture(const std::string& path);
