cmake_minimum_required(VERSION 3.16)
project(Lab1_Variant15_Qt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT DEFINED CMAKE_PREFIX_PATH AND WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.8.3/mingw_64")
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(Lab1Variant15
    main.cpp
    MainWindow.h
    MainWindow.cpp
    DrawingWidget.h
    DrawingWidget.cpp
    Matrix3.h
)

target_link_libraries(Lab1Variant15 PRIVATE Qt6::Widgets)
