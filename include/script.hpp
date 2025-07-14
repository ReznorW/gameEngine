#pragma once

#include <string>
#include <lua.hpp>

class Context;
class Object;

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
    void setOwner(Object* obj) {owner = obj;}

    // Execution
    void onStart();
    void update(float dt);

private:
    lua_State* L = nullptr;
    std::string lastError;
    std::string name;
    std::string sourceCode;
    Context* context = nullptr;
    Object* owner = nullptr;

    // Lua object bindings
    void registerObject(lua_State* L);
    static int obj_move(lua_State* L);
    static int obj_setPosition(lua_State* L);
    static int obj_getPosition(lua_State* L);
    static int obj_moveToward(lua_State* L);
    static int obj_rotate(lua_State* L);
    static int obj_setRotation(lua_State* L);
    static int obj_getRotation(lua_State* L);
    static int obj_lookAt(lua_State* L);
    static int obj_setScale(lua_State* L);
    static int obj_setUniformScale(lua_State* L);
    static int obj_getScale(lua_State* L);
    static int obj_destroy(lua_State* L);
    static int obj_checkCollision(lua_State* L);
    static int obj_getName(lua_State* L);

    // Lua function bindings
    static int lua_getObject(lua_State* L);
    static int lua_Object(lua_State* L);
    static int lua_createObject(lua_State* L);
    static int lua_destroyObject(lua_State* L);

    static int lua_isKeyPressed(lua_State* L);

    static int lua_getPlayerName(lua_State* L);
    static int lua_getPlayer(lua_State* L);

    // Lua utils
    static Context* getContext(lua_State* L);
};