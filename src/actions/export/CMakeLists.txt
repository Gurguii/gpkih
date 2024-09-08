cmake_minimum_required(VERSION 3.10)

project(gpkih VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# BEG - Export core 
add_library(gpkih_actions_export STATIC "execute.cpp" "help.cpp")
add_subdirectory("csv")

target_include_directories(gpkih_csv PUBLIC "${FMT_LIBRARY_INCLUDE}")
target_link_libraries(gpkih_actions_export PRIVATE gpkih_csv)
# END - Export core

# BEG - GPKIH_SQLITE 
if(GPKIH_SQLITE)
  add_compile_definitions("GPKIH_SQLITE")
  #set_property(TARGET gpkih_actions_export PROPERTY COMPILE_DEFINITIONS GPKIH_SQLITE)
  message(STATUS "SQLite3 support ENABLED")
  add_subdirectory("sqlite")
  target_link_libraries(gpkih_actions_export PRIVATE gpkih_sqlite)
  target_include_directories(gpkih_sqlite PUBLIC "${FMT_LIBRARY_INCLUDE}")
else()
  message(STATUS "SQLite3 support DISABLED")
endif()
# END - GPKIH_SQLITE

# BEG - GPKIH_PSQL
if(GPKIH_PSQL)
  add_compile_definitions("GPKIH_PSQL")
  #set_property(TARGET gpkih_actions_export PROPERTY COMPILE_DEFINITIONS GPKIH_PSQL)
  message(STATUS "PSQL support ENABLED")
  add_subdirectory("postgres")
  target_link_libraries(gpkih_actions_export PRIVATE gpkih_postgres)
  target_include_directories(gpkih_postgres PUBLIC "${FMT_LIBRARY_INCLUDE}")
else()
  message(STATUS "PSQL support DISABLED")
endif()
# END - GPKIH_PSQL

# BEG - GPKIH_MYSQL
if(GPKIH_MYSQL)
  add_compile_definitions("GPKIH_MYSQL")
  #set_property(TARGET gpkih_actions_export PROPERTY COMPILE_DEFINITIONS GPKIH_MYSQL)
  message(STATUS "MYSQL support ENABLED")
  add_subdirectory("mysql")
  target_link_libraries(gpkih_actions_export PRIVATE gpkih_mysql)
  target_include_directories(gpkih_mysql PUBLIC "${FMT_LIBRARY_INCLUDE}")
else()
  message(STATUS "MYSQL support DISABLED")
endif()
# END - GPKIH_MYSQL