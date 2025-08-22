cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME "GameOfLife")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

add_executable(GameOfLife main.cpp)

find_package(SDL2 2.32.2 EXACT REQUIRED)

find_package(SDL2_ttf REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf)
