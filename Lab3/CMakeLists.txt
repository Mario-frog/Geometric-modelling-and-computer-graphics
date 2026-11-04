cmake_minimum_required(VERSION 3.16)
project(Lab3_Dolphin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(lab3 main.cpp Canvas.cpp Canvas.h
        Canvas.cpp
        Canvas.cpp
        Canvas.cpp
        Canvas.cpp
        Canvas.cpp
        Canvas.cpp
        Canvas.cpp)
target_link_libraries(lab3 PRIVATE Qt6::Widgets)
