cmake_minimum_required(VERSION 3.16)
project(Lab5_Projection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(lab5 main.cpp Canvas.cpp Canvas.h Projection.h)
target_link_libraries(lab5 PRIVATE Qt6::Widgets)
