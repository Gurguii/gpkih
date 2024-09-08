cmake_minimum_required(VERSION 3.10)

project(gpkih VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(gpkih_sqlite STATIC "export.cpp" "SQLiteConnection.cpp")

find_library(SQLITE3_LIBRARY NAMES sqlite3 sqlite3.so PATHS "/usr/lib/")

if(NOT SQLITE3_LIBRARY)
  message(FATAL_ERROR "Couldn't find requested sqlite3 library")
else()
  message(STATUS "Found sqlite3 library: ${SQLITE3_LIBRARY}")
endif()

target_link_libraries(gpkih_sqlite PRIVATE ${SQLITE3_LIBRARY})