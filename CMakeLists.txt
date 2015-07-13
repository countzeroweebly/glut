cmake_minimum_required(VERSION 2.8)
project(openGL_00)
include_directories(.)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lGL -lGLU -lglut -ftest-coverage -fprofile-arcs")
set(SOURCE_EXE main.cpp)

add_executable(myGL ${SOURCE_EXE}) 
