cmake_minimum_required(VERSION 2.8)
project(openGL_06)
include_directories(.)

set(CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS} -lGL -lGLU -lglut -lpng -lzip -g -ftest-coverage -fprofile-arcs")
set(SOURCE_EXE main.cpp io.cpp core.cpp)
set(HEADER io.h core.h)

add_executable(myGL ${SOURCE_EXE}) 
