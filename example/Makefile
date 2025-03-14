CXX := g++
CXXFLAGS := -std=c++20 -I../include -Wall -Wextra -O
LIB_NAME := pinyin_ime
LIB_DIR := $(abspath ../lib)

# 默认目标：编译静态库示例程序
.PHONY: all
all: example_static example_shared

# 静态库示例程序
example_static: example.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -Wl,-Bstatic -L$(LIB_DIR) -l$(LIB_NAME) -Wl,-Bdynamic 

# 动态库示例程序
example_shared: example.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -Wl,-rpath=$(LIB_DIR) -L$(LIB_DIR) -l$(LIB_NAME)

# 清理生成文件
.PHONY: clean
clean:
	rm -f example_static example_shared