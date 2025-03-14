# 简介

基于字典树（Trie）结构实现的中文拼音输入法引擎库，采用UTF-8编码，支持词频排序、词频调整、拼音缩写搜索、记忆输入等功能。

项目主要目的是实现一个易用、易懂、易扩展的开源拼音输入法引擎，为不能使用主流输入法的场景提供选择。

# 注意事项

1. 库需要支持C++20的编译器进行编译，原因是项目中大量使用了 `std::string_view` 和 `std::span` 以减少内存使用。
1. 库的输入与输出均默认为UTF-8编码，且不支持其它编码。
1. 库采用异常作为错误处理方法，抛出的异常类型均为 `std::exception` 派生类。

# 快速开始

克隆此项目后，进入项目目录执行make即可。

```sh
git clone https://github.com/nspt/chinese_pinyin_ime.git
# 或者使用 gitee 链接
# git clone https://gitee.com/xiewei9608/chinese_pinyin_ime.git
cd chinese_pinyin_ime
make
```

make执行后，`lib` 目录下会生成静态库 `libpinyin_ime.a` 和动态库 `libpinyin_ime.so`，`example` 目录下会生成基于动态库的示例程序 `example_shared` 和基于静态库的示例程序 `example_static`。

可以通过示例程序源码 `example/example.cpp` 查看库的基本使用方法，默认词典库文件位于 `data/raw_dict_utf8.txt`。

需要交叉编译的场景请自行修改 `Makefile` 的 `CXX` 变量。

# 使用说明

待写

# 设计与实现

待写