cmake_minimum_required(VERSION 3.10)

project(glibs VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(gssl_x509 STATIC "x509.cpp")