# === Compiler and flags ===
CXX = g++
CXX_WIN = x86_64-w64-mingw32-g++

CXXFLAGS = -Wall -std=c++17 -g -Iinclude -Ilibs/glad/include -Ilibs/glfw -Ilibs/glm -Ilibs/imgui -Ilibs/imgui/backends
CXXFLAGS_WIN = -Wall -std=c++17 -g -Iinclude -Ilibs/glad/include -Ilibs/glfw/glfw-3.4.bin.WIN64/include -Ilibs/glm -Ilibs/imgui -Ilibs/imgui/backends

# === Linker flags ===
LDFLAGS = -Llibs/glfw/lib -lglfw -ldl -lGL
LDFLAGS_WIN = libs/glfw/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a -lopengl32 -lgdi32 -static-libgcc -static-libstdc++

# === Project structure ===
SRC_DIR := src
GLAD_SRC := libs/glad/src/glad.c
IMGUI_DIR := libs/imgui
IMGUI_BACKENDS := $(IMGUI_DIR)/backends

OBJ_DIR := obj
BIN_DIR := bin

TARGET_LINUX := $(BIN_DIR)/gameEngine
TARGET_WINDOWS := $(BIN_DIR)/gameEngine.exe

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
IMGUI_SRC := $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp \
             $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/imgui_demo.cpp \
             $(IMGUI_BACKENDS)/imgui_impl_glfw.cpp $(IMGUI_BACKENDS)/imgui_impl_opengl3.cpp

OBJ_FILES_LINUX := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
OBJ_IMGUI_LINUX := $(patsubst libs/%.cpp, $(OBJ_DIR)/%.o, $(IMGUI_SRC))
OBJ_GLAD_LINUX := $(OBJ_DIR)/glad.o

OBJ_FILES_WINDOWS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%_win.o, $(SRC_FILES))
OBJ_IMGUI_WIN := $(patsubst libs/%.cpp, $(OBJ_DIR)/%_win.o, $(IMGUI_SRC))
OBJ_GLAD_WIN := $(OBJ_DIR)/glad_win.o

# === Default target ===
all: $(TARGET_LINUX) $(TARGET_WINDOWS)

# === Linux build ===
$(TARGET_LINUX): $(OBJ_FILES_LINUX) $(OBJ_GLAD_LINUX) $(OBJ_IMGUI_LINUX)
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

# === Windows build ===
$(TARGET_WINDOWS): $(OBJ_FILES_WINDOWS) $(OBJ_GLAD_WIN) $(OBJ_IMGUI_WIN)
	@mkdir -p $(BIN_DIR)
	$(CXX_WIN) -o $@ $^ $(LDFLAGS_WIN)

$(OBJ_DIR)/%_win.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX_WIN) $(CXXFLAGS_WIN) -c $< -o $@

$(OBJ_DIR)/%_win.o: libs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX_WIN) $(CXXFLAGS_WIN) -c $< -o $@

$(OBJ_GLAD_WIN): $(GLAD_SRC)
	@mkdir -p $(OBJ_DIR)
	$(CXX_WIN) $(CXXFLAGS_WIN) -c $< -o $@

# === Cleanup ===
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
