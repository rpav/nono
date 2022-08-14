cmake_minimum_required(VERSION 3.17)

project(nono)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
add_executable(nono nono.cpp nono.hpp)
