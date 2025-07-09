#pragma once

#include <string>
#include <lua.hpp>

class Context;

class Script {
public:
    // Constructor
    Script(const std::string& scriptPath);
    Script(const Script& other);

    // Deconstructor
    ~Script();

    // Getters
    std::string getName() const {return name;}
    std::string getLastError() const {return lastError;}
    std::string getSource() const {return sourceCode;}

    void updateSource(const std::string& newCode);
    void saveToFile();
    void registerFunctions();

    // Setter
    void setName(const std::string& newName) {name = newName;}
    void setContext(Context* contextPtr);

    // Execution
    void update(float dt);

private:
    lua_State* L = nullptr;
    std::string lastError;
    std::string name;
    std::string sourceCode;
    Context* context = nullptr;

    // Lua function bindings
    static int lua_moveObject(lua_State* L);
    static int lua_setPosition(lua_State* L);
    static int lua_getPosition(lua_State* L);
    static int lua_rotateObject(lua_State* L);
    static int lua_setRotation(lua_State* L);
    static int lua_getRotation(lua_State* L);
    static int lua_setScale(lua_State* L);
    static int lua_getScale(lua_State* L);

    static int lua_createObject(lua_State* L);
    static int lua_destroyObject(lua_State* L);

    static int lua_isKeyPressed(lua_State* L);

    // Lua utils
    static Context* getContext(lua_State* L);
};