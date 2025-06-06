#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

// Load in shaders
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

// Constructor
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& name) 
        : name(name) {
    // Load in shaders from file path
    std::string vertexSrc = loadShaderSource(vertexPath);
    std::string fragmentSrc = loadShaderSource(fragmentPath);
    
    // Compile the shaders
    GLuint vertex = compile(GL_VERTEX_SHADER, vertexSrc.c_str());
    GLuint fragment = compile(GL_FRAGMENT_SHADER, fragmentSrc.c_str());

    // Initialize new shader program
    ID = glCreateProgram();

    // Attach and link compiled shaders to program
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    // Check if linking was successful
    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cerr << "Shader Linking Error: " << infoLog << "\n";
    }

    // Delete shaders (already loaded, no need for them anymore)
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// Deconstructor
Shader::~Shader() {
    glDeleteProgram(ID);
}

// Activates shader
void Shader::use() const {
    glUseProgram(ID);
}

// Getters
GLuint Shader::getID() const {
    return ID;
}

std::string Shader::getName() const {
    return name;
}

// Compiler
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

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' doesn't exist in shader.\n";
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: uniform '" << name << "' not found or optimized out.\n";
    } else {
        glUniform2fv(location, 1, &value[0]);
    }
}

void Shader::setFloat(const std::string& name, float value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' doesn't exist in shader.\n";
    }
    glUniform1f(location, value);
}

void Shader::setInt(const std::string& name, int value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' doesn't exist in shader.\n";
    }
    glUniform1i(location, value);
}

void Shader::setName(std::string& newName) {
    name = newName;
}