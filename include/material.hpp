#pragma once

#include <string>

// Material definition
class Material {
public:
    // Public vars
    float specular = 1.0f;
    float shininess = 32.0f;

    // Constructor
    Material(const std::string& path);

    // Deconstructor
    ~Material();

    // Getters
    unsigned int getDiffuseTexture() const {return diffuseTexture;}
    const std::string& getName() const {return name;}

    // Setters
    void setSpecular(float newSpecular) {specular = newSpecular;}
    void setShininess(float newShininess) {shininess = newShininess;}

    // Loader
    unsigned int loadTexture(const std::string& path);

    // Usage
    void bind(unsigned int diffuseSlot) const;

private:
    // Material data
    unsigned int diffuseTexture = 0;
    std::string name;
};
