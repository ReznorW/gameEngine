#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoords;
};

class Mesh {
public:
    Mesh(const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    void calculateBounds(const std::vector<Vertex>& vertices);
    void draw() const;

    const glm::vec3& getMinBounds() const {return minBounds;}
    const glm::vec3& getMaxBounds() const {return maxBounds;}
    std::string getName() const {return name;}

private:
    unsigned int VAO, VBO, EBO;
    size_t indexCount;
    std::string name;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

Mesh* loadVertFile(const std::string& filepath);
unsigned int loadTexture(const std::string& path);
