cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0079 NEW) # Allows linking libraries not built in this directory (line 87-88)

project(gpkih VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(GPKIH_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(GPKIH_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(GPKIH_INCLUDE_DIR "${GPKIH_SRC_DIR}")  # Include directory for all components
set(GURGUI_LIBRARIES gprintinglib glogginglib gutilslib gmemorylib)
#gmemorylib
set(GPKIH_LIBRARIES gpkih_mem_lib gpkih_profiles_lib gpkih_entities_lib gpkih_config_lib gpkih_actions_lib gpkih_db_lib gpkih_help_lib gpkih_parser_lib gpkih_formatter_lib gpkih_cli_lib)

# Platform-specific settings
function(configure_platform)
  if(WIN32)
    set(FMT_LIBRARY_PATH "C:/Program Files (x86)/FMT/lib")
    set(CMAKE_CXX_FLAGS "/O2 /EHsc")
    set(FMT_LIBRARY_NAMES "fmtd")
    set(FMT_LIBRARY_INCLUDE "C:/Program Files (x86)/FMT/include")
  else()
    set(FMT_LIBRARY_PATH "/usr/lib;/usr/local/lib")
    set(CMAKE_CXX_FLAGS "-O3")
    set(FMT_LIBRARY_NAMES "fmt")
    set(FMT_LIBRARY_INCLUDE "/usr/local/lib/fmt")
  endif()
endfunction()

configure_platform()

# Look for FMT
find_library(FMT_LIBRARY NAMES ${FMT_LIBRARY_NAMES} PATHS ${FMT_LIBRARY_PATH})

if(NOT FMT_LIBRARY)
  message(STATUS "FMT lib not found in system")
  # Set outpath for FMT source code
  set(fmtlib_src "${CMAKE_CURRENT_BINARY_DIR}/fmtlib")

  # Make sure it hasn't already been cloned
  if(NOT EXISTS "${fmtlib_src}")
    # Check if Git is available
    find_program(GIT git HINTS "C:/Program Files/Git" ENV PATH)

    if(NOT GIT)
      message(FATAL_ERROR "Git executable not found. Aborting.")
    endif()

    message(STATUS "cloning https://github.com/fmtlib/fmt.git to ${fmtlib_src}")
    
    # Clone fmtlib repository
    execute_process(COMMAND git clone --depth 1 https://github.com/fmtlib/fmt.git ${fmtlib_src})
  else()
    message(STATUS "fmtlib already exists, skipping git clone...")
  endif()

  # Add fmt to project
  add_subdirectory("${fmtlib_src}")

  set(FMT_LIBRARY_INCLUDE "${fmtlib_src}/include")
endif()

message(STATUS "FMT include dir: ${FMT_LIBRARY_INCLUDE}")

# Compile STATIC standalone libs
add_subdirectory("${GPKIH_SRC_DIR}/libs/utils")
add_subdirectory("${GPKIH_SRC_DIR}/libs/printing")
add_subdirectory("${GPKIH_SRC_DIR}/libs/logger")
add_subdirectory("${GPKIH_SRC_DIR}/libs/memory")

# Compile STATIC gpkih libs
add_subdirectory("${GPKIH_SRC_DIR}/profiles")
add_subdirectory("${GPKIH_SRC_DIR}/entities")
add_subdirectory("${GPKIH_SRC_DIR}/help")
add_subdirectory("${GPKIH_SRC_DIR}/cli")
add_subdirectory("${GPKIH_SRC_DIR}/db")
add_subdirectory("${GPKIH_SRC_DIR}/config")
add_subdirectory("${GPKIH_SRC_DIR}/actions")
add_subdirectory("${GPKIH_SRC_DIR}/parse")
add_subdirectory("${GPKIH_SRC_DIR}/experimental")

# Compile custom Buffer
add_library(gpkih_mem_lib STATIC "src/buffer.cpp")

# Add fmt include dirs to all libs (not all of them use it, but I've been a lot of hours trying to make it compile again, ill review it later.) 
foreach(target IN LISTS GURGUI_LIBRARIES GPKIH_LIBRARIES)
  target_include_directories(${target} PUBLIC "${FMT_LIBRARY_INCLUDE}")
endforeach()

add_executable(gpkih
    "${GPKIH_SRC_DIR}/gpkih.cpp"
)

target_include_directories(gpkih PUBLIC "${FMT_LIBRARY_INCLUDE}")

target_link_libraries(gpkih PUBLIC gpkih_entities_lib gpkih_profiles_lib gpkih_parser_lib gpkih_db_lib gpkih_actions_lib 
    gpkih_config_lib gpkih_help_lib gpkih_cli_lib gpkih_formatter_lib gpkih_mem_lib gprintinglib gutilslib glogginglib gmemorylib)

if(GPRINTING_ENABLE_DEBUGGING)
    message(STATUS "GPKIH Debugging capabilities ENABLED")
    set_property(TARGET gpkih ${GPKIH_LIBRARIES} PROPERTY COMPILE_DEFINITIONS GPRINTING_ENABLE_DEBUGGING)
else()
    message(STATUS "GPKIH Debugging capabilities DISABLED")
endif()

if(FMT_LIBRARY)
    # Compile with FMT found in the system
    message(STATUS "GPKIH will be compiled with found fmtlib")
    target_link_libraries(gpkih PRIVATE 
        ${GPKIH_LIBRARIES}
        FMT_LIBRARY
    )
else()
    # Use compiled FMT from github clone
    message(STATUS "GPKIH will be compiled with cloned fmtlib")
    target_link_libraries(gpkih PRIVATE 
        ${GPKIH_LIBRARIES}
        fmt::fmt
    )
endif()

return()