cmake_minimum_required(VERSION 3.8)
project(project2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        awget.cpp
        awget.h)

add_executable(project2 ${SOURCE_FILES})