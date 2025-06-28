cmake_minimum_required(VERSION 3.15)
project(assignment2
  LANGUAGES CXX
  VERSION 1.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SFML/CMakeLists.txt")
  message("${CMAKE_CURRENT_SOURCE_DIR}/SFML/CMakeLists.txt")
  message(FATAL_ERROR "Please initialize submodules using:\n  git submodule update --init --recursive")
endif()
set(SFML_BUILD_AUDIO FALSE)
set(SFML_BUILD_NETWORK FALSE)
add_subdirectory(third_party/SFML)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.cpp")
    message("${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.cpp")
    message(FATAL_ERROR "ImGui not found. Please initialize submodules using:\n git submodule update --init --recursive")
endif()
# imgui has no CmakeLists but imgui-sfml does and relies on these variables
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)

# Create SFML aliases that ImGui-SFML expects - because we build from subdirectory
add_library(SFML::Graphics ALIAS sfml-graphics)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui-sfml/CMakeLists.txt")
    message("${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui-sfml/CMakeLists.txt")
    message(FATAL_ERROR "ImGui-SFML not found. Please initialize submodules using:\n git submodule update --init --recursive")
endif()
add_subdirectory(third_party/imgui-sfml)