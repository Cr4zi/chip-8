cmake_minimum_required(VERSION 3.15)

project(chip-8)

find_package(raylib 3.0 REQUIRED)

set(CMAKE_C_STANDARD 11)

file(GLOB SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} raylib)
