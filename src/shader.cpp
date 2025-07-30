#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include "shader.hpp"

// === Constructor ===
Shader::Shader(const std::string& name, const bool& hasGeometry) 
        : name(name) {
    // Initialize shader
    ID = glCreateProgram();

    // Compile shader
    GLuint vShader = compile(GL_VERTEX_SHADER, loadShaderSource("shaders/" + name + "/vertex.glsl").c_str());
    GLuint fShader = compile(GL_FRAGMENT_SHADER, loadShaderSource("shaders/" + name + "/fragment.glsl").c_str());
    GLuint gShader = (hasGeometry) ? compile(GL_GEOMETRY_SHADER, loadShaderSource("shaders/" + name + "/geometry.glsl").c_str()) : 0;

    // Link shader
    glAttachShader(ID, vShader);
    glAttachShader(ID, fShader);
    if (hasGeometry) {glAttachShader(ID, gShader);}
    glLinkProgram(ID);

    // Check if linking was successful
    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cerr << "Shader Linking Error: " << infoLog << "\n";
    }

    // Clean up
    glDeleteShader(vShader);
    glDeleteShader(fShader);
    if (hasGeometry) {glDeleteShader(gShader);}
}

// === Deconstructor ===
Shader::~Shader() {
    glDeleteProgram(ID);
}

// === Usage ===
void Shader::use() const {
    glUseProgram(ID);
}

// === Getters ===
GLuint Shader::getID() const {
    return ID;
}

std::string Shader::getName() const {
    return name;
}

// === Setters ===
void Shader::setName(std::string& newName) {
    name = newName;
}

// === Uniform setters ===
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

// === Internal compilation ===
GLuint Shader::compile(GLenum type, const char* src) {
    // Creates shader
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);

    // Compiles shader
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compile Error: " << infoLog << "\n";
    }

    // Return final shader
    return shader;
}

// === Loader ===
std::string loadShaderSource(const std::string& filepath) {
    // Open the file at the given path
    std::ifstream file(filepath);
    
    // Check if the file opened successfully
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << "\n";
        return "";
    }

    // Create a string stream to store the file contents
    std::stringstream buffer;

    // Read the entire file into the buffer
    buffer << file.rdbuf();

    // Convert the buffer into a string and return it
    return buffer.str();
}