cmake_minimum_required(VERSION 3.21)
project(mint VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm CONFIG REQUIRED)

# find_package(GTest CONFIG REQUIRED)

# enable_testing()

add_subdirectory(src)
