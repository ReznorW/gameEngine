#pragma once

class Project {
public:
    std::string name;
    Resources* resources = nullptr;

    Project(const std::string& name, Resources* resources)
        : name(name), resources(resources) {}

};