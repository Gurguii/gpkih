cmake_minimum_required(VERSION 3.10)

project(gpkih VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(gpkih_postgres STATIC "export.cpp" "postgreSQLconnection.cpp")

find_library(PSQL_LIBRARY NAMES pq pq.so PATHS "/usr/lib/")

if(NOT PSQL_LIBRARY)
  message(FATAL_ERROR "Couldn't find requested libpq library")
else()
  message(STATUS "Found libpq library: ${PSQL_LIBRARY}")
endif()

target_link_libraries(gpkih_postgres PRIVATE ${PSQL_LIBRARY})