# === Compiler and flags ===
CXX = g++
CXX_WIN = x86_64-w64-mingw32-g++
CC = gcc
CC_WIN = x86_64-w64-mingw32-gcc

CXXFLAGS = -Wall -std=c++17 -g -Iinclude -Ilibs/glad/include -Ilibs/glfw -Ilibs/glm -Ilibs/imgui -Ilibs/imgui/backends -Ilibs/lua/src
CXXFLAGS_WIN = -Wall -std=c++17 -g -Iinclude -Ilibs/glad/include -Ilibs/glfw/glfw-3.4.bin.WIN64/include -Ilibs/glm -Ilibs/imgui -Ilibs/imgui/backends -Ilibs/lua/src

# === Linker flags ===
LDFLAGS = -Llibs/glfw/lib -lglfw -ldl -lGL
LDFLAGS_WIN = libs/glfw/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a -lopengl32 -lgdi32 -static-libgcc -static-libstdc++

# === Project structure ===
SRC_DIR := src
GLAD_SRC := libs/glad/src/glad.c
IMGUI_DIR := libs/imgui
IMGUI_BACKENDS := $(IMGUI_DIR)/backends
LUA_DIR := libs/lua/src

OBJ_DIR := obj
BIN_DIR := bin

TARGET_LINUX := $(BIN_DIR)/gameEngine
TARGET_WINDOWS := $(BIN_DIR)/gameEngine.exe

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
IMGUI_SRC := $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp \
             $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/imgui_demo.cpp \
             $(IMGUI_BACKENDS)/imgui_impl_glfw.cpp $(IMGUI_BACKENDS)/imgui_impl_opengl3.cpp
LUA_SRCS := \
    $(LUA_DIR)/lapi.c $(LUA_DIR)/lauxlib.c $(LUA_DIR)/lbaselib.c \
    $(LUA_DIR)/lcode.c $(LUA_DIR)/lcorolib.c $(LUA_DIR)/lctype.c \
    $(LUA_DIR)/ldblib.c $(LUA_DIR)/ldebug.c $(LUA_DIR)/ldo.c \
    $(LUA_DIR)/ldump.c $(LUA_DIR)/lfunc.c $(LUA_DIR)/lgc.c \
    $(LUA_DIR)/linit.c $(LUA_DIR)/liolib.c $(LUA_DIR)/llex.c \
    $(LUA_DIR)/lmathlib.c $(LUA_DIR)/lmem.c $(LUA_DIR)/loadlib.c \
    $(LUA_DIR)/lobject.c $(LUA_DIR)/lopcodes.c $(LUA_DIR)/loslib.c \
    $(LUA_DIR)/lparser.c $(LUA_DIR)/lstate.c $(LUA_DIR)/lstring.c \
    $(LUA_DIR)/lstrlib.c $(LUA_DIR)/ltable.c $(LUA_DIR)/ltablib.c \
    $(LUA_DIR)/ltm.c $(LUA_DIR)/lundump.c $(LUA_DIR)/lutf8lib.c \
    $(LUA_DIR)/lvm.c $(LUA_DIR)/lzio.c

OBJ_FILES_LINUX := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
OBJ_IMGUI_LINUX := $(patsubst libs/%.cpp, $(OBJ_DIR)/%.o, $(IMGUI_SRC))
OBJ_GLAD_LINUX := $(OBJ_DIR)/glad.o
OBJ_LUA_LINUX := $(patsubst $(LUA_DIR)/%.c, $(OBJ_DIR)/lua/linux/%.o, $(LUA_SRCS))
LUA_LIB_LINUX := libs/lua/liblua_linux.a

OBJ_FILES_WINDOWS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%_win.o, $(SRC_FILES))
OBJ_IMGUI_WIN := $(patsubst libs/%.cpp, $(OBJ_DIR)/%_win.o, $(IMGUI_SRC))
OBJ_GLAD_WIN := $(OBJ_DIR)/glad_win.o
OBJ_LUA_WIN := $(patsubst $(LUA_DIR)/%.c, $(OBJ_DIR)/lua/windows/%.o, $(LUA_SRCS))
LUA_LIB_WIN := libs/lua/liblua_win.a

# === Default target ===
all: $(TARGET_LINUX) $(TARGET_WINDOWS)

# === Linux build ===
$(TARGET_LINUX): $(OBJ_FILES_LINUX) $(OBJ_GLAD_LINUX) $(OBJ_IMGUI_LINUX) $(LUA_LIB_LINUX)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: libs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_GLAD_LINUX): $(GLAD_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/lua/linux/%.o: $(LUA_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -I$(LUA_DIR) -fPIC

$(LUA_LIB_LINUX): $(OBJ_LUA_LINUX)
	@mkdir -p libs/lua
	ar rcs $@ $^

# === Windows build ===
$(TARGET_WINDOWS): $(OBJ_FILES_WINDOWS) $(OBJ_GLAD_WIN) $(OBJ_IMGUI_WIN) $(LUA_LIB_WIN)
	@mkdir -p $(BIN_DIR)
	$(CXX_WIN) -o $@ $(OBJ_FILES_WINDOWS) $(OBJ_GLAD_WIN) $(OBJ_IMGUI_WIN) $(LUA_LIB_WIN) $(LDFLAGS_WIN)

$(OBJ_DIR)/%_win.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX_WIN) $(CXXFLAGS_WIN) -c $< -o $@

$(OBJ_DIR)/%_win.o: libs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX_WIN) $(CXXFLAGS_WIN) -c $< -o $@

$(OBJ_GLAD_WIN): $(GLAD_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX_WIN) $(CXXFLAGS_WIN) -c $< -o $@

$(OBJ_DIR)/lua/windows/%.o: $(LUA_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC_WIN) -c $< -o $@ -I$(LUA_DIR)

$(LUA_LIB_WIN): $(OBJ_LUA_WIN)
	@mkdir -p libs/lua
	ar rcs $@ $^

# === Cleanup ===
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LUA_LIB_LINUX) $(LUA_LIB_WIN)

.PHONY: all clean
