cmake_minimum_required(VERSION 3.16)
project(Lab2_Hypocycloid CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(MSVC)
  add_compile_options(/utf-8)   # українські рядки в UTF-8
endif()

# ---------------------------------------------------------------- SFML
# 1) якщо SFML вже встановлений (apt, brew, vcpkg) - використовуємо його;
# 2) інакше CMake сам завантажує SFML 2.6.1 і збирає його статично
#    (потрібен інтернет лише при першому конфігуруванні).
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

if(NOT SFML_FOUND)
  message(STATUS "SFML не знайдено в системі - завантажую SFML 2.6.1 (FetchContent)...")
  include(FetchContent)
  set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)           # статично: DLL не потрібні
  set(SFML_BUILD_AUDIO   OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(SFML_USE_STATIC_STD_LIBS ON CACHE BOOL "" FORCE)
  FetchContent_Declare(SFML
    URL https://github.com/SFML/SFML/archive/refs/tags/2.6.1.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(SFML)
endif()

# ----------------------------------------------------------- виконувані файли
add_executable(hypocycloid src/main.cpp)
target_link_libraries(hypocycloid PRIVATE sfml-graphics sfml-window sfml-system)
target_compile_definitions(hypocycloid PRIVATE ASSET_DIR="${CMAKE_SOURCE_DIR}/assets")

# консольний тест математичного ядра (без SFML)
add_executable(test_core src/test_core.cpp)

if(MINGW)   # повністю статичне лінкування: exe не потребує жодних MinGW DLL
  target_link_options(hypocycloid PRIVATE -static)
  target_link_options(test_core   PRIVATE -static)
endif()

# копіюємо шрифт поруч з exe
add_custom_command(TARGET hypocycloid POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:hypocycloid>/assets)
