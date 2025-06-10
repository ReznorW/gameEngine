#pragma once

#include <string>

// Texture definition
class Texture {
public:
    // Constructor
    Texture(const std::string& path);

    // Deconstructor
    ~Texture();

    // Getters
    unsigned int getID() const { return id; }
    const std::string& getName() const { return name; }

    // Usage
    void bind(unsigned int slot = 0) const;
private:
    // Texture data
    unsigned int id = 0;
    std::string name;
};
