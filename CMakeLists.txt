cmake_minimum_required(VERSION 2.8)
project(openGL_09_multithreading)
include_directories(.)

set(CMAKE_CXX_FLAGS "-pthread --std=c++11 ${CMAKE_CXX_FLAGS} -lGL -lGLU -lglut -lpng -lzip -g -ftest-coverage -fprofile-arcs")
set(SOURCE_EXE main.cpp io.cpp core.cpp)
set(HEADER core.h)

add_executable(myGL ${SOURCE_EXE}) 
