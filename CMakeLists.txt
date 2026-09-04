cmake_minimum_required(VERSION 3.21)
project(nomos VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 CONFIG REQUIRED)

# find_package(GTest CONFIG REQUIRED)

# enable_testing()

set(_DATASET_DIR "${CMAKE_SOURCE_DIR}/datasets/")

add_subdirectory(src)
