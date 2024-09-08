cmake_minimum_required(VERSION 3.10)

project(gpkih VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(gpkih_mysql STATIC "export.cpp" "MySqlConnection.cpp")

find_library(MYSQL_LIBRARY NAMES mariadb mariadb.so PATHS "/usr/lib/")

if(NOT MYSQL_LIBRARY)
  message(FATAL_ERROR "Couldn't find requested MYSQL library")
else()
  message(STATUS "Found MYSQL library: ${MYSQL_LIBRARY}")
endif()

target_link_libraries(gpkih_mysql PRIVATE ${MYSQL_LIBRARY})