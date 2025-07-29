#include <iostream>
#include <ostream>
#include <fstream>
#include <random>
#include <filesystem>
#include <glm/glm.hpp>

#include "script.hpp"
#include "object.hpp"
#include "window.hpp"

// === Constructors ===
Script::Script(const std::string& scriptPath) {
    L = luaL_newstate();
    luaL_openlibs(L);
    registerFunctions();

    name = std::filesystem::path(scriptPath).stem().string();

    std::ifstream file(scriptPath);
    if (!file.is_open()) {
        return; // Treat as new file
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    sourceCode = code;
    clearErrors();

    if (luaL_dostring(L, code.c_str()) != LUA_OK) {
        logError(std::string("[Lua Compile Error] ") + lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

Script::Script(const Script& other) {
    name = other.name;
    sourceCode = other.sourceCode;

    L = luaL_newstate();
    luaL_openlibs(L);
    registerFunctions();
    clearErrors();

    if (luaL_dostring(L, sourceCode.c_str()) != LUA_OK) {
        logError(std::string("[Lua Compile Error] ") + lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

// === Deconstructor ===
Script::~Script() {
    lua_close(L);
}

// === Setters ===
void Script::setName(const std::string& newName) {
    if (newName == name || newName.empty()) return;

    std::string oldPath = "assets/scripts/" + name + ".lua";
    std::string newPath = "assets/scripts/" + newName + ".lua";

    // Attempt to rename the file
    if (std::rename(oldPath.c_str(), newPath.c_str()) == 0) {
        name = newName;
    } else {
        std::cerr << "Failed to rename file from " << oldPath << " to " << newPath << '\n';
    }
}

void Script::setContext(Context* contextPtr) {
    context = contextPtr;
    lua_pushlightuserdata(L, static_cast<void*>(context));
    lua_setglobal(L, "__context");
}

// === Editing ===
void Script::updateSource(const std::string& newCode) {
    sourceCode = newCode;
    clearErrors();

    if (luaL_dostring(L, sourceCode.c_str()) != LUA_OK) {
        logError(std::string("[Lua Compile Error] ") + lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

void Script::saveToFile() {
    std::ofstream out("assets/scripts/" + name + ".lua");
    if (out.is_open()) {
        out << sourceCode;
        out.close();
    }
}

// === Error handling ===
void Script::logError(const std::string& error) {
    errorLog.push_back(error);
    if (errorLog.size() > 100) {
        errorLog.erase(errorLog.begin());
    }
}

void Script::clearErrors() {
    errorLog.clear();
}

bool Script::hasErrors() const {
    return !errorLog.empty();
}

// === Execution ===
void Script::onStart() {
    // Initialize self
    if (owner) {
        Object** udata = (Object**)lua_newuserdata(L, sizeof(Object*));
        *udata = owner;
        luaL_getmetatable(L, "Object");
        lua_setmetatable(L, -2);
        lua_setglobal(L, "self");
    }

    // Run onStart function
    lua_getglobal(L, "onStart");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            logError(std::string("[Lua Runtime Error] ") + lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1); // Not a function
    }
}

void Script::update(float dt) {
    if (!L) return;
    // Run update function
    lua_getglobal(L, "update");
    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, dt);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            logError(std::string("[Lua Runtime Error] ") + lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1); // Not a function
    }
}

// === Lua function registration ===
void Script::registerFunctions() {
    // Object
    registerObject(L);
    lua_register(L, "Object", lua_Object);

    // Player
    lua_register(L, "getPlayer", lua_getPlayer);
    lua_register(L, "getPlayerName", lua_getPlayerName);
    lua_register(L, "getPlayerSpeed", lua_getPlayerSpeed);
    lua_register(L, "getPlayerJump", lua_getPlayerJump);
    lua_register(L, "setPlayerSpeed", lua_setPlayerSpeed);
    lua_register(L, "setPlayerJump", lua_setPlayerJump);

    // Scene
    lua_register(L, "createObject", lua_createObject);
    lua_register(L, "destroyObject", lua_destroyObject);
    lua_register(L, "getObject", lua_getObject);
    lua_register(L, "getSkyColor", lua_getSkyColor);
    lua_register(L, "getGravity", lua_getGravity);
    lua_register(L, "getDrag", lua_getDrag);
    lua_register(L, "setSkyColor", lua_setSkyColor);
    lua_register(L, "setGravity", lua_setGravity);
    lua_register(L, "setDrag", lua_setDrag);

    // Input
    lua_register(L, "isKeyPressed", lua_isKeyPressed);
    lua_register(L, "isKeyPressedOnce", lua_isKeyPressedOnce);

    // Misc
    lua_register(L, "rand", lua_rand);
}

// === Lua object bindings ===
// --- Object registration ---
void Script::registerObject(lua_State* L) {
    luaL_newmetatable(L, "Object");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_Reg methods[] = {
        {"destroy", obj_destroy},
        {"getPosition", obj_getPosition},
        {"getRotation", obj_getRotation},
        {"getScale", obj_getScale},
        {"getName", obj_getName},
        {"setPosition", obj_setPosition},
        {"setRotation", obj_setRotation},
        {"setScale", obj_setScale},
        {"move", obj_move},
        {"rotate", obj_rotate},
        {"checkCollision", obj_checkCollision},
        {"moveToward", obj_moveToward},
        {"lookAt", obj_lookAt},
        {nullptr, nullptr}
    };

    luaL_setfuncs(L, methods, 0);
}

// --- Constructors ---
int Script::lua_Object(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 5 || args != 6) {
        return luaL_error(L, "Object expects 5 or 6 arguments");
    }

    // Check args
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3) || !lua_isstring(L, 4) || !lua_isstring(L, 5)) {
        return luaL_error(L, "Object expects (String, String, String, String, String) or (String, String, String, String, String, String)");
    }

    if (args == 6 && !lua_isstring(L, 6)) {
        return luaL_error(L, "Object expects (String, String, String, String, String) or (String, String, String, String, String, String)");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    const char* name = lua_tostring(L, 1);
    const char* model = lua_tostring(L, 2);
    const char* texture = lua_tostring(L, 3);
    // const char* shader = lua_tostring(L, 4);
    const char* script = lua_tostring(L, 5);
    const char* atThis = nullptr;
    if (args == 6) {atThis = lua_tostring(L, 6);}

    // Avoid duplicates
    if (context->playScene->getObject(name)) {
        std::string error = std::string("Object: Object with name '") + name + "' already exists";
        return luaL_error(L, "%s", error.c_str());
    }

    // Create and add the object
    std::shared_ptr<Object> obj = std::make_shared<Object>(std::string(name), std::string(model), std::string(texture), std::string(script), context->project->resources);

    // Apply the translation
    if (atThis) {
        Object* base = context->playScene->getObject(std::string(atThis));
        if (base) {
            obj->transform.position = base->transform.position;
            obj->transform.markDirty();
        } else {
            std::string error = std::string("Object: Base object '") + atThis + "' not found";
            return luaL_error(L, "%s", error.c_str());
        }
    }

    context->playScene->addObject(name, std::move(obj));

    // Execute script if exists
    Object* newObj = context->playScene->getObject(name);

    if (newObj && newObj->script) {
        newObj->script->setContext(context);
        newObj->script->onStart();
    }

    // Push results
    Object** udata = (Object**)lua_newuserdata(L, sizeof(Object*));
    *udata = newObj;
    luaL_getmetatable(L, "Object");
    lua_setmetatable(L, -2);
    return 1;
}

// --- Destructors ---
int Script::obj_destroy(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "destroy expects 0 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling destroy");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) return 0;

    // Perform function
    context->playScene->markForDeletion(obj->name);

    // Push results
    return 0;
}

// --- Getters ---
int Script::obj_getPosition(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "getPosition expects 0 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling getPosition");
    }

    // Perform function
    glm::vec3 pos = obj->transform.position;

    // Push results
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

int Script::obj_getRotation(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "getRotation expects 0 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling getRotation");
    }

    // Perform function
    glm::vec3 rot = obj->transform.rotation;

    // Push results
    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    lua_pushnumber(L, rot.z);
    return 3;
}

int Script::obj_getScale(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "getScale expects 0 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling getScale");
    }

    // Perform function
    glm::vec3 scale = obj->transform.scale;

    // Push results
    lua_pushnumber(L, scale.x);
    lua_pushnumber(L, scale.y);
    lua_pushnumber(L, scale.z);
    return 3;
}

int Script::obj_getName(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "getName expects 0 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling getName");
    }

    // Push results
    lua_pushstring(L, obj->name.c_str());
    return 1;
}

// --- Setters ---
int Script::obj_setPosition(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 4) {
        return luaL_error(L, "setPosition expects 3 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling setPosition");
    }

    if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "setPosition expects (Number, Number, Number)"); 
    }

    // Get args
    float x, y, z;
    x = static_cast<float>(lua_tonumber(L, 2));
    y = static_cast<float>(lua_tonumber(L, 3));
    z = static_cast<float>(lua_tonumber(L, 4));

    // Perform function
    obj->transform.position = glm::vec3(x, y, z);
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_setRotation(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 4) {
        return luaL_error(L, "setRotation expects 3 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling setRotation");
    }

    if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "setRotation expects (Number, Number, Number)"); 
    }

    // Get args
    float pitch, yaw, roll;
    pitch = static_cast<float>(lua_tonumber(L, 2));
    yaw = static_cast<float>(lua_tonumber(L, 3));
    roll = static_cast<float>(lua_tonumber(L, 4));

    // Perform function
    obj->transform.setRotation(glm::vec3(pitch, yaw, roll));
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_setScale(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 2 || args != 4) {
        return luaL_error(L, "setScale expects 1 or 3 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling setScale");
    }

    if (!lua_isnumber(L, 2)) {
        return luaL_error(L, "setScale expects (Number) or (Number, Number, Number)"); 
    }

    if (args == 4 && (!lua_isnumber(L, 3) || !lua_isnumber(L, 4))) {
        return luaL_error(L, "setScale expects (Number) or (Number, Number, Number)");
    }

    // Get args and perform function
    float x = static_cast<float>(lua_tonumber(L, 2));
    if (args == 4) {
        float y = static_cast<float>(lua_tonumber(L, 3));
        float z = static_cast<float>(lua_tonumber(L, 4));
        obj->transform.scale = glm::vec3(x, y, z);
    } else {
        obj->transform.scale = glm::vec3(x, x, x);
    }
    obj->transform.markDirty();

    // Push results
    return 0;
}

// --- Physics ---
int Script::obj_move(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 4) {
        return luaL_error(L, "move expects 3 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling move");
    }

    if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "move expects (Number, Number, Number)"); 
    }

    // Get args
    float x, y, z;
    x = static_cast<float>(lua_tonumber(L, 2));
    y = static_cast<float>(lua_tonumber(L, 3));
    z = static_cast<float>(lua_tonumber(L, 4));

    // Perform function
    obj->transform.velocity += glm::vec3(x, y, z);
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_rotate(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 4) {
        return luaL_error(L, "rotate expects 3 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling rotate");
    }

    if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "rotate expects (Number, Number, Number)"); 
    }

    // Get args
    float pitch, yaw, roll;
    pitch = static_cast<float>(lua_tonumber(L, 2));
    yaw = static_cast<float>(lua_tonumber(L, 3));
    roll = static_cast<float>(lua_tonumber(L, 4));

    // Perform function
    obj->transform.setRotation(obj->transform.rotation + glm::vec3(pitch, yaw, roll));
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_checkCollision(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 2) {
        return luaL_error(L, "checkCollision expects 1 argument");
    }

    // Check args
    Object* objA = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!objA) {
        return luaL_error(L, "Invalid Object calling checkCollision");
    }

    if (!lua_isstring(L, 2) && !luaL_testudata(L, 2, "Object")) {
        return luaL_error(L, "checkCollision expects (String) or (Object)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    Object* objB = nullptr;
    if (lua_isstring(L, 2)) {
        objB = context->playScene->getObject(lua_tostring(L, 2));
    } else {
        objB = *(Object**)luaL_checkudata(L, 2, "Object");
    }

    // Perform function
    bool colliding = areIntersecting(*objA, *objB);

    // Push result
    lua_pushboolean(L, colliding);
    return 1;
}

// --- Trackers ---
int Script::obj_moveToward(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 3) {
        return luaL_error(L, "moveToward expects 2 arguments");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling moveToward");
    }

    if ((!lua_isstring(L, 2) && !luaL_testudata(L, 2, "Object")) || !lua_isnumber(L, 3)) {
        return luaL_error(L, "moveToward expects (String, Number) or (Object, Number)"); 
    }
    
    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    Object* target = nullptr;
    if (lua_isstring(L, 2)) {
        target = context->playScene->getObject(lua_tostring(L, 2));
    } else {
        target = *(Object**)luaL_checkudata(L, 2, "Object");
    }
    float speed = static_cast<float>(lua_tonumber(L, 3));

    // Perform function
    glm::vec3 objPos = obj->transform.position;
    glm::vec3 targetPos = target->transform.position;
    glm::vec3 delta = targetPos - objPos;

    float dist2 = glm::length(delta);
    if (dist2 < 0.0001f) {
        obj->transform.velocity = glm::vec3(0.0f);
    } else {
        glm::vec3 direction = glm::normalize(delta);
        glm::vec3 velocity = direction * speed;
        obj->transform.velocity = velocity;
    }
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_lookAt(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 2) {
        return luaL_error(L, "lookAt expects 1 argument");
    }

    // Check args
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    if (!obj) {
        return luaL_error(L, "Invalid Object calling lookAt");
    }

    if (!lua_isstring(L, 2) && !luaL_testudata(L, 2, "Object")) {
        return luaL_error(L, "lookAt expects (String) or (Object)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    Object* target = nullptr;
    if (lua_isstring(L, 2)) {
        target = context->playScene->getObject(lua_tostring(L, 2));
    } else {
        target = *(Object**)luaL_checkudata(L, 2, "Object");
    }

    // Perform function
    glm::vec3 dir = glm::normalize(target->transform.position - obj->transform.position);
    float yaw = glm::degrees(atan2(dir.x, dir.z));

    obj->transform.setRotation(glm::vec3(obj->transform.rotation.x, yaw, obj->transform.rotation.z));
    obj->transform.markDirty();

    // Push results
    return 0;
}

// === Lua player bindings ===
int Script::lua_getPlayer(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 0) {
        return luaL_error(L, "getPlayer expects 0 arguments");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Perform function
    for (const auto& obj : context->playScene->getObjects()) {
        if (obj && obj->isPlayer) {
            // Push results
            Object** udata = (Object**)lua_newuserdata(L, sizeof(Object*));
            *udata = obj;
            luaL_getmetatable(L, "Object");
            lua_setmetatable(L, -2);
            return 1;
        }
    }
    return luaL_error(L, "getPlayer could not find player object");
}

int Script::lua_getPlayerName(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 0) {
        return luaL_error(L, "getPlayerName expects 0 arguments");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Perform function
    for (const auto& obj : context->playScene->getObjects()) {
        if (obj && obj->isPlayer) {
            // Push results
            lua_pushstring(L, obj->name.c_str());
            return 1;
        }
    }
    return luaL_error(L, "getPlayerName could not find player object");
}

int Script::lua_getPlayerSpeed(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 0) {
        return luaL_error(L, "getPlayerSpeed expects 0 arguments");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Push results
    lua_pushnumber(L, context->playScene->playerSpeed);
    return 1;
}

int Script::lua_getPlayerJump(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 0) {
        return luaL_error(L, "getPlayerJump expects 0 arguments");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Push results
    lua_pushnumber(L, context->playScene->playerJump);
    return 1;
}

int Script::lua_setPlayerSpeed(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "setPlayerSpeed expects 1 argument");
    }

    // Check args
    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "setPlayerSpeed expects (Number)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    float speed = static_cast<float>(lua_tonumber(L, 1));

    // Perform function
    context->playScene->playerSpeed = speed;
    
    // Push results
    return 0;
}

int Script::lua_setPlayerJump(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "setPlayerJump expects 1 argument");
    }

    // Check args
    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "setPlayerJump expects (Number)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    float jump = static_cast<float>(lua_tonumber(L, 1));

    // Perform function
    context->playScene->playerJump = jump;
    
    // Push results
    return 0;
}

// === Lua scene bindings ===
int Script::lua_createObject(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 5 && args != 6) {
        std::string error = std::string("createObject expects 5 or 6 arguments not " + std::to_string(args));
        return luaL_error(L, "%s", error.c_str());
    }

    // Check args
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3) || !lua_isstring(L, 4) || !lua_isstring(L, 5)) {
        return luaL_error(L, "createObject expects (String, String, String, String, String) or (String, String, String, String, String, String)");
    }

    if (args == 6 && !lua_isstring(L, 6)) {
        return luaL_error(L, "createObject expects (String, String, String, String, String) or (String, String, String, String, String, String)");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    const char* name = lua_tostring(L, 1);
    const char* model = lua_tostring(L, 2);
    const char* texture = lua_tostring(L, 3);
    // const char* shader = lua_tostring(L, 4);
    const char* script = lua_tostring(L, 5);
    const char* atThis = nullptr;
    if (args == 6) {atThis = lua_tostring(L, 6);}

    // Avoid duplicates
    if (context->playScene->getObject(name)) {
        std::string error = std::string("createObject: Object with name '") + name + "' already exists";
        return luaL_error(L, "%s", error.c_str());
    }

    // Create and add the object
    std::shared_ptr<Object> obj = std::make_shared<Object>(std::string(name), std::string(model), std::string(texture), std::string(script), context->project->resources);

    if (atThis) {
        Object* base = context->playScene->getObject(std::string(atThis));
        if (base) {
            obj->transform.position = base->transform.position;
            obj->transform.markDirty();
        } else {
            std::string error = std::string("Object: Base object '") + atThis + "' not found";
            return luaL_error(L, "%s", error.c_str());
        }
    }

    context->playScene->addObject(name, std::move(obj));

    // Execute script if exists
    Object* newObj = context->playScene->getObject(name);

    if (newObj && newObj->script) {
        newObj->script->setContext(context);
        newObj->script->onStart();
    }

    return 0;
}

int Script::lua_destroyObject(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "destroyObject expects 1 argument");
    }

    // Check args
    if (!lua_isstring(L, 1) && !luaL_testudata(L, 1, "Object")) {
        return luaL_error(L, "destroyObject expects (String) or (Object)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    Object* obj = nullptr;
    if (lua_isstring(L, 2)) {
        obj = context->playScene->getObject(lua_tostring(L, 2));
    } else {
        obj = *(Object**)luaL_checkudata(L, 2, "Object");
    }

    // Perform function
    context->playScene->markForDeletion(obj->name);

    // Push results
    return 0;
}

int Script::lua_getObject(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "getObject expects 1 argument");
    }

    // Check args
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "getObject expects (String)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    const char* name = lua_tostring(L, 1);

    // Perform function
    Object* obj = context->playScene->getObject(name);
    if (!obj) {
        std::string error = std::string("getObject: Object '") + name + "' not found";
        return luaL_error(L, "%s", error.c_str());
    }

    // Push results
    Object** udata = (Object**)lua_newuserdata(L, sizeof(Object*));
    *udata = obj;
    luaL_getmetatable(L, "Object");
    lua_setmetatable(L, -2);
    return 1;
}

int Script::lua_getSkyColor(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 0) {
        return luaL_error(L, "getSkyColor expects 0 arguments");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Push results
    lua_pushnumber(L, context->playScene->skyColor.x);
    lua_pushnumber(L, context->playScene->skyColor.y);
    lua_pushnumber(L, context->playScene->skyColor.z);
    lua_pushnumber(L, context->playScene->skyColor.w);
    return 1;
}

int Script::lua_getGravity(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 0) {
        return luaL_error(L, "getGravity expects 0 arguments");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Push results
    lua_pushnumber(L, context->playScene->gravity.x);
    lua_pushnumber(L, context->playScene->gravity.y);
    lua_pushnumber(L, context->playScene->gravity.z);
    return 1;
}

int Script::lua_getDrag(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 0) {
        return luaL_error(L, "getDrag expects 0 arguments");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Push results
    lua_pushnumber(L, context->playScene->drag);
    return 1;
}

int Script::lua_setSkyColor(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 4) {
        return luaL_error(L, "setSkyColor expects 4 arguments");
    }

    // Check args
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "setSkyColor expects (Number, Number, Number, Number)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    float red, green, blue, alpha;
    red = static_cast<float>(lua_tonumber(L, 1));
    if (red < 0 || red > 255) {
        return luaL_error(L, "setSkyColor: Number out of range"); 
    }
    green = static_cast<float>(lua_tonumber(L, 2));
    if (green < 0 || green > 255) {
        return luaL_error(L, "setSkyColor: Number out of range"); 
    }
    blue = static_cast<float>(lua_tonumber(L, 3));
    if (blue < 0 || blue > 255) {
        return luaL_error(L, "setSkyColor: Number out of range"); 
    }
    alpha = static_cast<float>(lua_tonumber(L, 4));
    if (alpha < 0 || alpha > 255) {
        return luaL_error(L, "setSkyColor: Number out of range"); 
    }

    // Perform function
    context->playScene->skyColor = glm::vec4(red, green, blue, alpha);
    
    // Push results
    return 0;
}

int Script::lua_setGravity(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1 || args != 3) {
        return luaL_error(L, "setGravity expects 1 or 3 arguments");
    }

    // Check args
    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "setGravity expects (Number) or (Number, Number, Number)"); 
    }

    if (args == 3 && (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))) {
        return luaL_error(L, "setGravity expects (Number) or (Number, Number, Number)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args and perform function
    float x = static_cast<float>(lua_tonumber(L, 1));
    if (args == 4) {
        float y = static_cast<float>(lua_tonumber(L, 2));
        float z = static_cast<float>(lua_tonumber(L, 3));
        context->playScene->gravity = glm::vec3(x, y, z);
    } else {
        context->playScene->gravity = glm::vec3(0, x, 0);
    }
    
    // Push results
    return 0;
}

int Script::lua_setDrag(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "setDrag expects 1 argument");
    }

    // Check args
    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "setDrag expects (Number)"); 
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}

    // Get args
    float drag = static_cast<float>(lua_tonumber(L, 1));
    if (drag < 0.0f || drag > 1.0f) {
        return luaL_error(L, "setDrag: Number out of range"); 
    }

    // Perform function
    context->playScene->drag = drag;
    
    // Push results
    return 0;
}

// === Lua input bindings ===
int Script::lua_isKeyPressed(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "isKeyPressed expects 1 argument");
    }

    // Check args
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "isKeyPressed expects (String)");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}
    GLFWwindow* window = context->window->getGLFWwindow();

    // Get args
    const char* keyStr = lua_tostring(L, 1);

    // Perform function
    int key = getKeyFromString(keyStr);
    if (key == -1) {
        std::string error = std::string("isKeyPressed: key '") + keyStr + "' does not exist";
        return luaL_error(L, "%s", error.c_str());
    }
    int state = glfwGetKey(window, key);

    // Push results
    lua_pushboolean(L, state == GLFW_PRESS);
    return 1;
}

int Script::lua_isKeyPressedOnce(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1) {
        return luaL_error(L, "isKeyPressed expects 1 argument");
    }

    // Check args
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "isKeyPressed expects (String)");
    }

    // Get context
    Context* context = getContext(L);
    if (!context || !context->playScene) {return 0;}
    GLFWwindow* window = context->window->getGLFWwindow();

    // Get args
    const char* keyStr = lua_tostring(L, 1);

    // Perform function
    int key = getKeyFromString(keyStr);
    if (key == -1) {
        std::string error = std::string("isKeyPressedOnce: key '") + keyStr + "' does not exist";
        return luaL_error(L, "%s", error.c_str());
    }
    static std::unordered_map<int, bool> keyStates;
    int state = glfwGetKey(window, key);
    bool isDown = (state == GLFW_PRESS);
    bool wasDown = keyStates[key];
    keyStates[key] = isDown;

    // Push results
    lua_pushboolean(L, isDown && !wasDown);
    return 1;
}

// === Lua misc bindings ===
int Script::lua_rand(lua_State* L) {
    // Check number of args
    int args = lua_gettop(L);
    if (args != 1 || args != 2) {
        return luaL_error(L, "rand expects 1 or 2 arguments");
    }

    // Check args
    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "rand expects (Number) or (Number, Number)"); 
    }

    if (args == 2 && !lua_isnumber(L, 2)) {
        return luaL_error(L, "rand expects (Number) or (Number, Number)"); 
    }

    // Get args and perform function
    std::random_device rd;
    std::mt19937 gen(rd());

    if (args == 1) {
        int max = static_cast<int>(lua_tonumber(L, 1));
        std::uniform_int_distribution<> dist(0, max);
        lua_pushinteger(L, dist(gen));
        return 1;
    } else {
        int min = static_cast<int>(lua_tonumber(L, 1));
        int max = static_cast<int>(lua_tonumber(L, 2));
        std::uniform_int_distribution<> dist(min, max);
        lua_pushinteger(L, dist(gen));
        return 1;
    }
}

// === Lua utils ===
Context* Script::getContext(lua_State* L) {
    lua_getglobal(L, "__context");
    Context* context = static_cast<Context*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return context;
}

int Script::getKeyFromString(const char* keyStr) {
    if (!keyStr) return -1;

    if (strcmp(keyStr, "A") == 0) return GLFW_KEY_A;
    else if (strcmp(keyStr, "B") == 0) return GLFW_KEY_B;
    else if (strcmp(keyStr, "C") == 0) return GLFW_KEY_C;
    else if (strcmp(keyStr, "D") == 0) return GLFW_KEY_D;
    else if (strcmp(keyStr, "E") == 0) return GLFW_KEY_E;
    else if (strcmp(keyStr, "F") == 0) return GLFW_KEY_F;
    else if (strcmp(keyStr, "G") == 0) return GLFW_KEY_G;
    else if (strcmp(keyStr, "H") == 0) return GLFW_KEY_H;
    else if (strcmp(keyStr, "I") == 0) return GLFW_KEY_I;
    else if (strcmp(keyStr, "J") == 0) return GLFW_KEY_J;
    else if (strcmp(keyStr, "K") == 0) return GLFW_KEY_K;
    else if (strcmp(keyStr, "L") == 0) return GLFW_KEY_L;
    else if (strcmp(keyStr, "M") == 0) return GLFW_KEY_M;
    else if (strcmp(keyStr, "N") == 0) return GLFW_KEY_N;
    else if (strcmp(keyStr, "O") == 0) return GLFW_KEY_O;
    else if (strcmp(keyStr, "P") == 0) return GLFW_KEY_P;
    else if (strcmp(keyStr, "Q") == 0) return GLFW_KEY_Q;
    else if (strcmp(keyStr, "R") == 0) return GLFW_KEY_R;
    else if (strcmp(keyStr, "S") == 0) return GLFW_KEY_S;
    else if (strcmp(keyStr, "T") == 0) return GLFW_KEY_T;
    else if (strcmp(keyStr, "U") == 0) return GLFW_KEY_U;
    else if (strcmp(keyStr, "V") == 0) return GLFW_KEY_V;
    else if (strcmp(keyStr, "W") == 0) return GLFW_KEY_W;
    else if (strcmp(keyStr, "X") == 0) return GLFW_KEY_X;
    else if (strcmp(keyStr, "Y") == 0) return GLFW_KEY_Y;
    else if (strcmp(keyStr, "Z") == 0) return GLFW_KEY_Z;

    else if (strcmp(keyStr, "0") == 0) return GLFW_KEY_0;
    else if (strcmp(keyStr, "1") == 0) return GLFW_KEY_1;
    else if (strcmp(keyStr, "2") == 0) return GLFW_KEY_2;
    else if (strcmp(keyStr, "3") == 0) return GLFW_KEY_3;
    else if (strcmp(keyStr, "4") == 0) return GLFW_KEY_4;
    else if (strcmp(keyStr, "5") == 0) return GLFW_KEY_5;
    else if (strcmp(keyStr, "6") == 0) return GLFW_KEY_6;
    else if (strcmp(keyStr, "7") == 0) return GLFW_KEY_7;
    else if (strcmp(keyStr, "8") == 0) return GLFW_KEY_8;
    else if (strcmp(keyStr, "9") == 0) return GLFW_KEY_9;

    else if (strcmp(keyStr, "Space") == 0) return GLFW_KEY_SPACE;
    else if (strcmp(keyStr, "Enter") == 0) return GLFW_KEY_ENTER;
    else if (strcmp(keyStr, "Tab") == 0) return GLFW_KEY_TAB;
    else if (strcmp(keyStr, "Backspace") == 0) return GLFW_KEY_BACKSPACE;

    else if (strcmp(keyStr, "Left") == 0) return GLFW_KEY_LEFT;
    else if (strcmp(keyStr, "Right") == 0) return GLFW_KEY_RIGHT;
    else if (strcmp(keyStr, "Up") == 0) return GLFW_KEY_UP;
    else if (strcmp(keyStr, "Down") == 0) return GLFW_KEY_DOWN;

    else if (strcmp(keyStr, "LeftShift") == 0) return GLFW_KEY_LEFT_SHIFT;
    else if (strcmp(keyStr, "RightShift") == 0) return GLFW_KEY_RIGHT_SHIFT;
    else if (strcmp(keyStr, "LeftCtrl") == 0) return GLFW_KEY_LEFT_CONTROL;
    else if (strcmp(keyStr, "RightCtrl") == 0) return GLFW_KEY_RIGHT_CONTROL;
    else if (strcmp(keyStr, "LeftAlt") == 0) return GLFW_KEY_LEFT_ALT;
    else if (strcmp(keyStr, "RightAlt") == 0) return GLFW_KEY_RIGHT_ALT;

    else if (strcmp(keyStr, "F1") == 0) return GLFW_KEY_F1;
    else if (strcmp(keyStr, "F2") == 0) return GLFW_KEY_F2;
    else if (strcmp(keyStr, "F3") == 0) return GLFW_KEY_F3;
    else if (strcmp(keyStr, "F4") == 0) return GLFW_KEY_F4;
    else if (strcmp(keyStr, "F5") == 0) return GLFW_KEY_F5;
    else if (strcmp(keyStr, "F6") == 0) return GLFW_KEY_F6;
    else if (strcmp(keyStr, "F7") == 0) return GLFW_KEY_F7;
    else if (strcmp(keyStr, "F8") == 0) return GLFW_KEY_F8;
    else if (strcmp(keyStr, "F9") == 0) return GLFW_KEY_F9;
    else if (strcmp(keyStr, "F10") == 0) return GLFW_KEY_F10;
    else if (strcmp(keyStr, "F11") == 0) return GLFW_KEY_F11;
    else if (strcmp(keyStr, "F12") == 0) return GLFW_KEY_F12;

    return -1;
}
