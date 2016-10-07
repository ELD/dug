cmake_minimum_required(VERSION 3.6)
project(dug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES src/main.cpp headers/includes.h headers/constants.h headers/structures.h headers/functions.h src/functions.cpp)
add_executable(dug ${SOURCE_FILES})