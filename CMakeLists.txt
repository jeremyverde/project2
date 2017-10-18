cmake_minimum_required(VERSION 3.8)
project(project2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES_AWGET
        awget.cpp
        awget.h)

set(SOURCE_FILES_SS
        ss.cpp
        awget.h)

add_executable(awget ${SOURCE_FILES_AWGET})
add_executable(ss ${SOURCE_FILES_SS})