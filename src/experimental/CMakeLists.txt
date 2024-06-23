cmake_minimum_required(VERSION 3.10)

project(gpkih VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(GLOB sources "implementation/*.cpp")

add_library(gpkih_formatter_lib STATIC "${sources}")