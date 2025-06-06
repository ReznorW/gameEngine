#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Shader {
public:
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& name);
    ~Shader();
    void use() const;

    unsigned int getID() const;
    std::string getName() const;

    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void setName(std::string& newName);

private:
    unsigned int ID;
    std::string name;
    unsigned int compile(unsigned int type, const char* src);
};

std::string loadShaderSource(const std::string& filepath);
