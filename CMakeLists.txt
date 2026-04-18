cmake_minimum_required(VERSION 3.23)

project(chinese_pinyin_ime)

option(BUILD_EXAMPLE "Build example" ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(chinese_pinyin_ime)
target_compile_features(chinese_pinyin_ime PUBLIC cxx_std_20)
target_sources(chinese_pinyin_ime
PRIVATE
    source/ime.cpp
    source/candidates.cpp
    source/dict_item.cpp
    source/dict.cpp
    source/pinyin.cpp
    source/query.cpp
PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
    include/ime.h
)

if (BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
