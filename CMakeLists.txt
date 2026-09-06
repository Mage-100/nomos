cmake_minimum_required(VERSION 3.21)
project(nomos VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Eigen3 CONFIG REQUIRED)

# find_package(GTest CONFIG REQUIRED)

# enable_testing()

set(_DATASET_DIR "${CMAKE_SOURCE_DIR}/datasets/")

add_subdirectory(src)
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
