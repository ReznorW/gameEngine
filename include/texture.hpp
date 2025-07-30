#pragma once

#include <string>

// Texture definition
class Texture {
public:
    // Constructor
    Texture(const std::string& path);
    Texture(const unsigned int shadowSize, const std::vector<float>& shadowCascadeLevels);
    Texture(const unsigned int shadowSize);

    // Deconstructor
    ~Texture();

    // Getters
    unsigned int getID() const {return id;}
    const std::string& getName() const {return name;}

    // Usage
    void bind(unsigned int slot = 0) const;
    void bindArray(unsigned int slot) const;
    void bindCube(unsigned int slot) const;
private:
    // Texture data
    unsigned int id = 0;
    std::string name;
};
