cmake_minimum_required(VERSION 3.10)

project(gpkih VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(gpkih_entities_lib STATIC "implementation/conv.cpp" "implementation/entities.cpp" "implementation/subj_utils.cpp")