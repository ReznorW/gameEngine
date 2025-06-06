#pragma once
#include <string>

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    unsigned int getID() const { return id; }
    const std::string& getName() const { return name; }

private:
    unsigned int id = 0;
    std::string name;
};
