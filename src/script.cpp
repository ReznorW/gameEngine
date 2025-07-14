#include <iostream>
#include <ostream>
#include <fstream>
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
        std::cerr << "Failed to open script file: " << scriptPath << std::endl;
        return;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    sourceCode = code;

    if (luaL_dostring(L, code.c_str()) != LUA_OK) {
        lastError = lua_tostring(L, -1);
        std::cerr << "[Lua Compile Error] " << lastError << std::endl;
        lua_pop(L, 1);
    }
}

Script::Script(const Script& other) {
    name = other.name;
    sourceCode = other.sourceCode;

    L = luaL_newstate();
    luaL_openlibs(L);
    registerFunctions();

    if (luaL_dostring(L, sourceCode.c_str()) != LUA_OK) {
        lastError = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
}

// === Deconstructor ===
Script::~Script() {
    lua_close(L);
}

void Script::setContext(Context* contextPtr) {
    context = contextPtr;
    lua_pushlightuserdata(L, static_cast<void*>(context));
    lua_setglobal(L, "__context");
}

void Script::updateSource(const std::string& newCode) {
    sourceCode = newCode;

    if (luaL_dostring(L, sourceCode.c_str()) != LUA_OK) {
        lastError = lua_tostring(L, -1);
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

    lua_getglobal(L, "onStart");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            lastError = lua_tostring(L, -1);
            std::cerr << "[Lua Runtime Error in onStart] " << lastError << std::endl;
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1); // Not a function
    }
}

void Script::update(float dt) {
    if (!L) return;
    lua_getglobal(L, "update");
    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, dt);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lastError = lua_tostring(L, -1);
            std::cerr << "[Lua Runtime Error] " << lastError << std::endl;
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1); // Not a function
    }
}

// === Lua function bindings ===
void Script::registerFunctions() {
    // Object
    registerObject(L);
    lua_register(L, "getObject", lua_getObject);
    lua_register(L, "Object", lua_Object);

    // Scene management
    lua_register(L, "createObject", lua_createObject);
    lua_register(L, "destroyObject", lua_destroyObject);

    // Input management
    lua_register(L, "isKeyPressed", lua_isKeyPressed);

    lua_register(L, "getPlayerName", lua_getPlayerName);
    lua_register(L, "getPlayer", lua_getPlayer);
}

void Script::registerObject(lua_State* L) {
    luaL_newmetatable(L, "Object");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_Reg methods[] = {
        {"move", obj_move},
        {"setPosition", obj_setPosition},
        {"getPosition", obj_getPosition},
        {"moveToward", obj_moveToward},
        {"rotate", obj_rotate},
        {"setRotation", obj_setRotation},
        {"getRotation", obj_getRotation},
        {"lookAt", obj_lookAt},
        {"setScale", obj_setScale},
        {"getScale", obj_getScale},
        {"destroy", obj_destroy},
        {"checkCollision", obj_checkCollision},
        {"getName", obj_getName},
        {nullptr, nullptr}
    };

    luaL_setfuncs(L, methods, 0);
}

int Script::lua_getObject(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    Context* context = getContext(L);
    if (!context || !context->scene) return 0;

    Object* obj = context->scene->getObject(name);
    if (!obj) return 0;

    Object** udata = (Object**)lua_newuserdata(L, sizeof(Object*));
    *udata = obj;

    luaL_getmetatable(L, "Object");
    lua_setmetatable(L, -2);
    return 1;
}

int Script::obj_move(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);

    // Perform function
    obj->transform.velocity += glm::vec3(x, y, z);
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_setPosition(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);

    // Perform function
    obj->transform.position = glm::vec3(x, y, z);
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_getPosition(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");

    // Perform function
    glm::vec3 pos = obj->transform.position;

    // Push results
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

int Script::obj_moveToward(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");

    // Get context
    Context* context = getContext(L);
    if (!context || !context->scene) return 0;

    Object* target = nullptr;
    float speed = 0.0f;

    if (lua_gettop(L) == 3) {
        if (lua_isstring(L, 2)) {
            const char* targetName = lua_tostring(L, 2);
            target = context->scene->getObject(targetName);
        } else if (luaL_testudata(L, 2, "Object")) {
            target = *(Object**)luaL_checkudata(L, 2, "Object");
        } else {
            return luaL_error(L, "moveToward expects (String, Number) or (Object, Number)");
        }

        speed = luaL_checknumber(L, 3);
    } else {
        return luaL_error(L, "moveToward expects 2 arguments");
    }

    if (!target) return 0;

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

int Script::obj_rotate(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);

    // Perform function
    obj->transform.rotation = obj->transform.rotation + glm::vec3(x, y, z);
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_setRotation(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);

    // Perform function
    obj->transform.rotation = glm::vec3(x, y, z);
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_getRotation(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");

    // Perform function
    glm::vec3 rot = obj->transform.rotation;

    // Push results
    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    lua_pushnumber(L, rot.z);
    return 3;
}

int Script::obj_lookAt(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");

    // Get the context
    Context* context = getContext(L);
    if (!context || !context->scene) return 0;

    Object* target = nullptr;

    if (lua_isstring(L, 2)) {
        const char* targetName = lua_tostring(L, 2);
        target = context->scene->getObject(targetName);
    } else if (luaL_testudata(L, 2, "Object")) {
        target = *(Object**)luaL_checkudata(L, 2, "Object");
    } else {
        return luaL_error(L, "lookAt expects (String) or (Object)");
    }

    if (!target) return 0;

    // Perform function
    glm::vec3 dir = glm::normalize(target->transform.position - obj->transform.position);
    float yaw = glm::degrees(atan2(dir.x, dir.z));

    obj->transform.rotation.y = yaw;
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_setScale(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");
    float x = luaL_checknumber(L, 2);

    // Perform function
    if (lua_gettop(L) == 4) {
        float y = luaL_checknumber(L, 3);
        float z = luaL_checknumber(L, 4);
        obj->transform.scale = glm::vec3(x, y, z);
    } else {
        obj->transform.scale = glm::vec3(x, x, x);
    }
    obj->transform.markDirty();

    // Push results
    return 0;
}

int Script::obj_getScale(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");

    // Perform function
    glm::vec3 scale = obj->transform.scale;

    // Push results
    lua_pushnumber(L, scale.x);
    lua_pushnumber(L, scale.y);
    lua_pushnumber(L, scale.z);
    return 3;
}

int Script::lua_createObject(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* model = luaL_checkstring(L, 2);
    const char* texture = luaL_checkstring(L, 3);
    const char* shader = luaL_checkstring(L, 4);
    const char* scriptName = luaL_checkstring(L, 5);

    Context* context = getContext(L);
    if (!context || !context->scene) return 0;

    const char* atThis = nullptr;
    if (lua_gettop(L) >= 6 && lua_isstring(L, 6)) {
        atThis = lua_tostring(L, 6);
    }

    // Avoid duplicates
    if (context->scene->getObject(name)) {
        std::cerr << "createObject: Object with name '" << name << "' already exists.\n";
        return 0;
    }

    // Create and add the object
    std::shared_ptr<Object> obj = std::make_shared<Object>(name, model, texture, shader, scriptName, context->scene->getResources());

    if (atThis) {
        Object* base = context->scene->getObject(atThis);
        if (base) {
            obj->transform.position = base->transform.position;
            obj->transform.markDirty();
        } else {
            std::cerr << "createObject: Base object '" << atThis << "' not found.\n";
        }
    }

    context->scene->addObject(name, std::move(obj));

    // Execute script if exists
    Object* newObj = context->scene->getObject(name);

    if (newObj && newObj->script) {
        newObj->script->setContext(context);
        newObj->script->onStart();
    }

    return 0;
}

int Script::lua_Object(lua_State* L) {
    // Get parameters
    const char* name = luaL_checkstring(L, 1);
    const char* model = luaL_checkstring(L, 2);
    const char* texture = luaL_checkstring(L, 3);
    const char* shader = luaL_checkstring(L, 4);
    const char* scriptName = luaL_checkstring(L, 5);

    // Get context
    Context* context = getContext(L);
    if (!context || !context->scene) return 0;

    // Check for translation
    const char* atThis = nullptr;
    if (lua_gettop(L) >= 6 && lua_isstring(L, 6)) {
        atThis = lua_tostring(L, 6);
    }

    // Avoid duplicates
    if (context->scene->getObject(name)) {
        std::cerr << "createObject: Object with name '" << name << "' already exists.\n";
        return 0;
    }

    // Create and add the object
    std::shared_ptr<Object> obj = std::make_shared<Object>(name, model, texture, shader, scriptName, context->scene->getResources());

    // Apply the translation
    if (atThis) {
        Object* base = context->scene->getObject(atThis);
        if (base) {
            obj->transform.position = base->transform.position;
            obj->transform.markDirty();
        } else {
            std::cerr << "createObject: Base object '" << atThis << "' not found.\n";
        }
    }

    context->scene->addObject(name, std::move(obj));

    // Execute script if exists
    Object* newObj = context->scene->getObject(name);

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

int Script::lua_destroyObject(lua_State* L) {
    // Get the context
    Context* context = getContext(L);
    if (!context || !context->scene) return 0;

    // Get parameters
    Object* obj = nullptr;

    if (lua_isstring(L, 1)) {
        const char* name = lua_tostring(L, 1);
        obj = context->scene->getObject(name);
    } else if (luaL_testudata(L, 1, "Object")) {
        obj = *(Object**)luaL_checkudata(L, 1, "Object");
    } else {
        return luaL_error(L, "destroyObject expects (String) or (Object)");
    }

    if (obj) {
        context->scene->markForDeletion(obj->name);
    }

    return 0;
}

int Script::obj_destroy(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");

    // Get context
    Context* context = getContext(L);
    if (!context || !context->scene) return 0;

    // Perform function
    context->scene->markForDeletion(obj->name);

    // Push results
    return 0;
}

int Script::obj_checkCollision(lua_State* L) {
    // Get parameters
    Object* objA = *(Object**)luaL_checkudata(L, 1, "Object");

    // Get context
    Context* context = getContext(L);
    if (!context || !context->scene) return 0;

    Object* objB = nullptr;

    if (lua_isstring(L, 2)) {
        const char* nameB = lua_tostring(L, 2);
        objB = context->scene->getObject(nameB);
        if (!objB) {
            std::string msg = std::string("Object '") + nameB + "' not found";
            lua_pushstring(L, msg.c_str());
            lua_error(L);
            return 0;
        }
    } else if (luaL_testudata(L, 2, "Object")) {
        objB = *(Object**)luaL_checkudata(L, 2, "Object");
    } else {
        return luaL_error(L, "checkCollision expects (String) or (Object)");
    }

    // Perform function
    bool colliding = areIntersecting(*objA, *objB);

    // Push result
    lua_pushboolean(L, colliding);
    return 1;
}

int Script::lua_isKeyPressed(lua_State* L) {
    const char* keyStr = luaL_checkstring(L, 1);
    if (!keyStr) {
        lua_pushboolean(L, false);
        return 1;
    }

    Context* context = getContext(L);
    GLFWwindow* window = context->window->getGLFWwindow();

    int key = -1;

    if (strcmp(keyStr, "A") == 0) key = GLFW_KEY_A;
    else if (strcmp(keyStr, "B") == 0) key = GLFW_KEY_B;
    else if (strcmp(keyStr, "C") == 0) key = GLFW_KEY_C;
    else if (strcmp(keyStr, "D") == 0) key = GLFW_KEY_D;
    else if (strcmp(keyStr, "E") == 0) key = GLFW_KEY_E;
    else if (strcmp(keyStr, "F") == 0) key = GLFW_KEY_F;
    else if (strcmp(keyStr, "G") == 0) key = GLFW_KEY_G;
    else if (strcmp(keyStr, "H") == 0) key = GLFW_KEY_H;
    else if (strcmp(keyStr, "I") == 0) key = GLFW_KEY_I;
    else if (strcmp(keyStr, "J") == 0) key = GLFW_KEY_J;
    else if (strcmp(keyStr, "K") == 0) key = GLFW_KEY_K;
    else if (strcmp(keyStr, "L") == 0) key = GLFW_KEY_L;
    else if (strcmp(keyStr, "M") == 0) key = GLFW_KEY_M;
    else if (strcmp(keyStr, "N") == 0) key = GLFW_KEY_N;
    else if (strcmp(keyStr, "O") == 0) key = GLFW_KEY_O;
    else if (strcmp(keyStr, "P") == 0) key = GLFW_KEY_P;
    else if (strcmp(keyStr, "Q") == 0) key = GLFW_KEY_Q;
    else if (strcmp(keyStr, "R") == 0) key = GLFW_KEY_R;
    else if (strcmp(keyStr, "S") == 0) key = GLFW_KEY_S;
    else if (strcmp(keyStr, "T") == 0) key = GLFW_KEY_T;
    else if (strcmp(keyStr, "U") == 0) key = GLFW_KEY_U;
    else if (strcmp(keyStr, "V") == 0) key = GLFW_KEY_V;
    else if (strcmp(keyStr, "W") == 0) key = GLFW_KEY_W;
    else if (strcmp(keyStr, "X") == 0) key = GLFW_KEY_X;
    else if (strcmp(keyStr, "Y") == 0) key = GLFW_KEY_Y;
    else if (strcmp(keyStr, "Z") == 0) key = GLFW_KEY_Z;

    else if (strcmp(keyStr, "0") == 0) key = GLFW_KEY_0;
    else if (strcmp(keyStr, "1") == 0) key = GLFW_KEY_1;
    else if (strcmp(keyStr, "2") == 0) key = GLFW_KEY_2;
    else if (strcmp(keyStr, "3") == 0) key = GLFW_KEY_3;
    else if (strcmp(keyStr, "4") == 0) key = GLFW_KEY_4;
    else if (strcmp(keyStr, "5") == 0) key = GLFW_KEY_5;
    else if (strcmp(keyStr, "6") == 0) key = GLFW_KEY_6;
    else if (strcmp(keyStr, "7") == 0) key = GLFW_KEY_7;
    else if (strcmp(keyStr, "8") == 0) key = GLFW_KEY_8;
    else if (strcmp(keyStr, "9") == 0) key = GLFW_KEY_9;

    else if (strcmp(keyStr, "Space") == 0) key = GLFW_KEY_SPACE;
    else if (strcmp(keyStr, "Enter") == 0) key = GLFW_KEY_ENTER;
    else if (strcmp(keyStr, "Tab") == 0) key = GLFW_KEY_TAB;
    else if (strcmp(keyStr, "Backspace") == 0) key = GLFW_KEY_BACKSPACE;
    else if (strcmp(keyStr, "Left") == 0) key = GLFW_KEY_LEFT;
    else if (strcmp(keyStr, "Right") == 0) key = GLFW_KEY_RIGHT;
    else if (strcmp(keyStr, "Up") == 0) key = GLFW_KEY_UP;
    else if (strcmp(keyStr, "Down") == 0) key = GLFW_KEY_DOWN;

    else if (strcmp(keyStr, "LeftShift") == 0) key = GLFW_KEY_LEFT_SHIFT;
    else if (strcmp(keyStr, "RightShift") == 0) key = GLFW_KEY_RIGHT_SHIFT;
    else if (strcmp(keyStr, "LeftCtrl") == 0) key = GLFW_KEY_LEFT_CONTROL;
    else if (strcmp(keyStr, "RightCtrl") == 0) key = GLFW_KEY_RIGHT_CONTROL;
    else if (strcmp(keyStr, "LeftAlt") == 0) key = GLFW_KEY_LEFT_ALT;
    else if (strcmp(keyStr, "RightAlt") == 0) key = GLFW_KEY_RIGHT_ALT;

    else if (strcmp(keyStr, "F1") == 0) key = GLFW_KEY_F1;
    else if (strcmp(keyStr, "F2") == 0) key = GLFW_KEY_F2;
    else if (strcmp(keyStr, "F3") == 0) key = GLFW_KEY_F3;
    else if (strcmp(keyStr, "F4") == 0) key = GLFW_KEY_F4;
    else if (strcmp(keyStr, "F5") == 0) key = GLFW_KEY_F5;
    else if (strcmp(keyStr, "F6") == 0) key = GLFW_KEY_F6;
    else if (strcmp(keyStr, "F7") == 0) key = GLFW_KEY_F7;
    else if (strcmp(keyStr, "F8") == 0) key = GLFW_KEY_F8;
    else if (strcmp(keyStr, "F9") == 0) key = GLFW_KEY_F9;
    else if (strcmp(keyStr, "F10") == 0) key = GLFW_KEY_F10;
    else if (strcmp(keyStr, "F11") == 0) key = GLFW_KEY_F11;
    else if (strcmp(keyStr, "F12") == 0) key = GLFW_KEY_F12;

    if (key == -1) {
        lua_pushboolean(L, false);
        return 1;
    }

    int state = glfwGetKey(window, key);
    lua_pushboolean(L, state == GLFW_PRESS);
    return 1;
}

int Script::obj_getName(lua_State* L) {
    // Get parameters
    Object* obj = *(Object**)luaL_checkudata(L, 1, "Object");

    // Push results
    lua_pushstring(L, obj->name.c_str());
    return 1;
}

int Script::lua_getPlayerName(lua_State* L) {
    Context* context = getContext(L);

    for (const auto& obj : context->scene->getObjects()) {
        if (obj && obj->isPlayer) {
            lua_pushstring(L, obj->name.c_str());
            return 1;
        }
    }

    lua_pushnil(L); // Player not found
    return 1;
}

int Script::lua_getPlayer(lua_State* L) {
    Context* context = getContext(L);

    for (const auto& obj : context->scene->getObjects()) {
        if (obj && obj->isPlayer) {
            // Push results
            Object** udata = (Object**)lua_newuserdata(L, sizeof(Object*));
            *udata = obj;
            luaL_getmetatable(L, "Object");
            lua_setmetatable(L, -2);
            return 1;
        }
    }

    lua_pushnil(L); // Player not found
    return 1;
}

//=== Lua utils ===
Context* Script::getContext(lua_State* L) {
    lua_getglobal(L, "__context");
    Context* context = static_cast<Context*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return context;
}