cmake_minimum_required(VERSION 3.8)
project(steppingStone)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        ss.cpp
        ss.h)

add_executable(steppingStone ${SOURCE_FILES})