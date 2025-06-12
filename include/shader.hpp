#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// Shader definition
class Shader {
public:
    // Constructor
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& name);

    // Deconstructor
    ~Shader();

    // Usage
    void use() const;

    // Getters
    unsigned int getID() const;
    std::string getName() const;

    // Setters 
    void setName(std::string& newName);

    // Uniform setters
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void setBool(const std::string &name, bool value) const;

private:
    // Shader data
    unsigned int ID;
    std::string name;

    // Internal compilation
    unsigned int compile(unsigned int type, const char* src);
};

// Loader
std::string loadShaderSource(const std::string& filepath);
