CXX := g++
CXXFLAGS := -std=c++20 -I../include -Wall -Wextra -O3
LIB_NAME := pinyin_ime
LIB_DIR := ../lib

# 默认目标：编译静态库测试程序
.PHONY: all
all: test_static test_shared

# 静态库测试程序
test_static: test.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -L$(LIB_DIR) -l$(LIB_NAME)

# 动态库测试程序
test_shared: test.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR) -l$(LIB_NAME)

# 清理生成文件
.PHONY: clean
clean:
	rm -f test_static test_shared