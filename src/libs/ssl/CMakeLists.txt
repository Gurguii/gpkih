cmake_minimum_required(VERSION 3.10)

project(glibs VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(GLOB_RECURSE pkey_sources LIST_DIRECTORIES true "pkey/*.cpp")
file(GLOB_RECURSE x509_sources LIST_DIRECTORIES true "x509/*.cpp")

set(sources
	${pkey_sources} 
	${x509_sources}
)

add_library(gssl STATIC "${sources}")