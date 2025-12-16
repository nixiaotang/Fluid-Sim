# Compiler settings
CXX      := g++
CC       := gcc
CXXFLAGS := -std=c++20 -Wall -Wextra -g -Iinclude -fdiagnostics-color=always -MMD -MP
CFLAGS   := -Wall -Wextra -g -Iinclude -fdiagnostics-color=always -MMD -MP

# Linker flags
LDFLAGS := -Llib lib/libglfw.3.4.dylib \
           -framework OpenGL

# Detect sources
CPP_SRC := $(wildcard *.cpp) $(wildcard src/*.cpp)
C_SRC   := src/glad.c

# Build output folder (only .o and .d go here)
BUILD := build

# Object + dependency files
OBJ := $(CPP_SRC:%.cpp=$(BUILD)/%.o) \
       $(C_SRC:%.c=$(BUILD)/%.o)

DEP := $(OBJ:.o=.d)

# Final binary in project ROOT
TARGET := app

# Default
all: $(TARGET)

# Link final executable in root folder
$(TARGET): $(OBJ)
	@mkdir -p $(BUILD)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile C++ → build/file.o + build/file.d
$(BUILD)/%.o: %.cpp
	@mkdir -p $(BUILD)/$(dir $*)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C → build/file.o + build/file.d
$(BUILD)/%.o: %.c
	@mkdir -p $(BUILD)/$(dir $*)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD) $(TARGET)

re: clean all

.PHONY: all clean re

# Include dependency files
-include $(DEP)
