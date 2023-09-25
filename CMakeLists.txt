cmake_minimum_required(VERSION 3.22)

project(CodEx)

include_directories(${PROJECT_SOURCE_DIR})

file(GLOB SOURCES "src/*.cpp")
add_executable(codex ${SOURCES})
#add_executable(codex src/main.cpp src/parse.cpp)