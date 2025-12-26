cmake_minimum_required(VERSION 3.20)

project(ITQ VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(itq INTERFACE)

target_include_directories(itq INTERFACE include)
target_compile_options(itq INTERFACE -Wall -Wextra -Wpedantic)

install(DIRECTORY include/ DESTINATION include/itq)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()