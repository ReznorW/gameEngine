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
    std::vector<std::string> getErrorLog() const {return errorLog;}
    std::string getSource() const {return sourceCode;}

    // Setters
    void setName(const std::string& newName);
    void setOwner(Object* obj) {owner = obj;}
    void setContext(Context* contextPtr);
    void setErrorLog(std::vector<std::string> newErrorLog) {errorLog = newErrorLog;}

    // Editing
    void updateSource(const std::string& newCode);
    void saveToFile();

    // Error handling
    void logError(const std::string& error);
    void clearErrors();
    bool hasErrors() const;

    // Execution
    void onStart();
    void update(float dt);

    // Lua function registration
    void registerFunctions();

private:
    lua_State* L = nullptr;
    std::vector<std::string> errorLog;
    std::string name;
    std::string sourceCode;
    Context* context = nullptr;
    Object* owner = nullptr;

    // Lua object bindings
    void registerObject(lua_State* L);
    static int lua_Object(lua_State* L);

    static int obj_destroy(lua_State* L);
    static int obj_getPosition(lua_State* L);
    static int obj_getRotation(lua_State* L);
    static int obj_getScale(lua_State* L);
    static int obj_getName(lua_State* L);
    static int obj_setPosition(lua_State* L);
    static int obj_setRotation(lua_State* L);
    static int obj_setScale(lua_State* L);
    static int obj_move(lua_State* L);
    static int obj_rotate(lua_State* L);
    static int obj_checkCollision(lua_State* L);
    static int obj_moveToward(lua_State* L);
    static int obj_lookAt(lua_State* L);

    // Lua player bindings
    static int lua_getPlayer(lua_State* L);
    static int lua_getPlayerName(lua_State* L);
    static int lua_getPlayerSpeed(lua_State* L);
    static int lua_getPlayerJump(lua_State* L);
    static int lua_setPlayerSpeed(lua_State* L);
    static int lua_setPlayerJump(lua_State* L);

    // Lua scene bindings
    static int lua_createObject(lua_State* L);
    static int lua_destroyObject(lua_State* L);
    static int lua_getObject(lua_State* L);
    static int lua_getSkyColor(lua_State* L);
    static int lua_getGravity(lua_State* L);
    static int lua_getDrag(lua_State* L);
    static int lua_setSkyColor(lua_State* L);
    static int lua_setGravity(lua_State* L);
    static int lua_setDrag(lua_State* L);

    // Lua input bindings
    static int lua_isKeyPressed(lua_State* L);
    static int lua_isKeyPressedOnce(lua_State* L);

    // Lua misc bindings
    static int lua_rand(lua_State* L);

    // Lua utils
    static Context* getContext(lua_State* L);
    static int getKeyFromString(const char* keyStr);
};