CXX := g++
CXXFLAGS := -std=c++20 -Iinclude -Wall -Wextra -O
LIB_NAME := pinyin_ime
SOURCE_DIR := source
BUILD_DIR := build
LIB_DIR := lib
EXAMPLE_DIR := example

# 定义静态库和动态库的源文件及目标文件路径
SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
STATIC_OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/static_objs/%.o, $(SOURCES))
SHARED_OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/shared_objs/%.o, $(SOURCES))

.PHONY: all lib_static lib_shared example clean

# 默认目标：同时生成静态库和动态库，再编译示例程序
all: lib_static lib_shared example

# 生成静态库
lib_static: $(LIB_DIR)/lib$(LIB_NAME).a

# 生成动态库
lib_shared: $(LIB_DIR)/lib$(LIB_NAME).so

# 示例程序构建（依赖两种库）
example: lib_static lib_shared
	$(MAKE) -C $(EXAMPLE_DIR) clean
	$(MAKE) -C $(EXAMPLE_DIR)

# 清理所有生成文件
clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)
	$(MAKE) -C $(EXAMPLE_DIR) clean

# 静态库规则（使用普通目标文件）
$(LIB_DIR)/lib$(LIB_NAME).a: $(STATIC_OBJECTS)
	@mkdir -p $(@D)
	ar rcs $@ $^

# 动态库规则（需 -fPIC 和 -shared）
$(LIB_DIR)/lib$(LIB_NAME).so: $(SHARED_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) -shared $^ -o $@

# 静态库目标文件编译（无 -fPIC）
$(BUILD_DIR)/static_objs/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 动态库目标文件编译（需 -fPIC）
$(BUILD_DIR)/shared_objs/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@