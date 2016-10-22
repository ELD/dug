cmake_minimum_required(VERSION 3.5)
project(dug)

set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME ON)

find_package(BOOST COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_EXE_LINKER_FLAGS "-lboost_system -lboost_program_options")

set(SOURCE_FILES src/main.cpp headers/includes.h headers/constants.h headers/structures.h headers/functions.h src/functions.cpp)
add_executable(dug ${SOURCE_FILES})
