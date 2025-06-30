#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>

// Forward declaration
class Scene;

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
    // Constructors
    Mesh(const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    Mesh(const Mesh& other);
    
    // Deconstructor
    ~Mesh();

    // Getters
    const glm::vec3& getMinBounds() const {return minBounds;}
    const glm::vec3& getMaxBounds() const {return maxBounds;}
    std::string getName() const {return name;}
    const std::vector<Vertex>& getVertices() const {return vertices;}
    const std::vector<unsigned int>& getIndices() const {return indices;}

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

    // Vertex data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Internal setup
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

// Loaders
Mesh* loadVertFile(const std::string& filepath);
unsigned int loadTexture(const std::string& path);
bool saveMesh(const std::string& name, const Mesh& mesh, const std::string& filepath, Scene& scene);
