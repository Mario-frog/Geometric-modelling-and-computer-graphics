cmake_minimum_required(VERSION 3.16)
project(Lab4_Dolphin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(lab4 main.cpp Canvas.cpp Canvas.h)
target_link_libraries(lab4 PRIVATE Qt6::Widgets)
