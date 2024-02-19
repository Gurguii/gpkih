cmake_minimum_required(VERSION 3.10)

project(gpkih VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(WIN32)
        set(FMT_LIBRARY_PATH "C:/Program Files (x86)/FMT/lib")
        set(FMT_LIBRARY_INCLUDE "C:/Program Files (x86)/FMT/include")
        set(CMAKE_CXX_FLAGS "/O2 /EHsc")
        set(FMT_LIBRARY_NAMES "fmtd")
else()
        set(FMT_LIBRARY_PATH "/usr/lib;/usr/local/lib")
        set(CMAKE_CXX_FLAGS "-O3")
        set(FMT_LIBRARY_NAMES "fmt")
endif()



set(GPKIH_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(GPKIH_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${SOURCE_DIR}")

# Compile different gpkih components as static libraries

# Lib 1 - config_management
set (gpkih_config_definition
        "${GPKIH_SRC_DIR}/config_management.hpp")
set (gpkih_config_implementation
        "${GPKIH_SRC_DIR}/config_management.cpp"
)
add_library(gpkih_config_management_lib STATIC "${gpkih_config_implementation}")
target_include_directories(gpkih_config_management_lib PUBLIC
        "${INCLUDE_DIR}"
        "${FMT_LIBRARY_INCLUDE}"
)

# Lib 2 - actions
set(gpkih_actions_src "${GPKIH_SRC_DIR}/actions")
set (gpkih_actions_definition
        "${GPKIH_SRC_DIR}/actions/actions.hpp")
set (gpkih_actions_implementation
        "${gpkih_actions_src}/build.cpp"
        "${gpkih_actions_src}/gencrl.cpp"
        "${gpkih_actions_src}/revoke.cpp"
        "${gpkih_actions_src}/init.cpp"
        "${gpkih_actions_src}/list.cpp"
        "${gpkih_actions_src}/remove.cpp"
)
add_library(gpkih_actions_lib STATIC "${gpkih_actions_implementation}")
target_include_directories(gpkih_actions_lib PUBLIC
        "${INCLUDE_DIR}"
        "${FMT_LIBRARY_INCLUDE}"
)

# Lib 3 - parse
set(gpkih_parse_src "${GPKIH_SRC_DIR}/parse")
set(gpkih_parse_definition "${gpkih_parse_src}/parser.hpp")
set(gpkih_parse_implementation
        "${gpkih_parse_src}/build.cpp"
        "${gpkih_parse_src}/config.cpp"
        "${gpkih_parse_src}/gencrl.cpp"
        "${gpkih_parse_src}/revoke.cpp"
        "${gpkih_parse_src}/init.cpp"
        "${gpkih_parse_src}/list.cpp"
        "${gpkih_parse_src}/genkey.cpp"
        "${gpkih_parse_src}/remove.cpp"
        "${gpkih_parse_src}/parser.cpp"
)
add_library(gpkih_parse_lib STATIC "${gpkih_parse_implementation}")
target_include_directories(gpkih_parse_lib PUBLIC
        "${INCLUDE_DIR}"
        "${FMT_LIBRARY_INCLUDE}"
)

# Lib 4 - database
set(gpkih_db_src "${GPKIH_SRC_DIR}/db")
set(gpkih_db_definition "${gpkih_db_src}/database.hpp")
set(gpkih_db_implementation 
        "${gpkih_db_src}/profiles.cpp"
        "${gpkih_db_src}/entities.cpp"
)
add_library(gpkih_db_lib STATIC "${gpkih_db_implementation}")
target_include_directories(gpkih_db_lib PUBLIC
        "${INCLUDE_DIR}"
        "${FMT_LIBRARY_INCLUDE}"
)

# Lib 5 - help
set(gpkih_help_src "${GPKIH_SRC_DIR}/help")
set(gpkih_help_definition "${gpkih_help_src}/help.hpp")
set(gpkih_help_implementation
        "${gpkih_help_src}/help.cpp"
        "${gpkih_help_src}/get.cpp"
        "${gpkih_help_src}/help_build.cpp"
        "${gpkih_help_src}/help_create_pack.cpp"
        "${gpkih_help_src}/help_gencrl.cpp"
        "${gpkih_help_src}/help_init.cpp"
        "${gpkih_help_src}/help_remove.cpp"
        "${gpkih_help_src}/help_revoke.cpp"
)
add_library(gpkih_help_lib STATIC "${gpkih_help_implementation}")
target_include_directories(gpkih_help_lib PUBLIC
        "${INCLUDE_DIR}"
        "${FMT_LIBRARY_INCLUDE}"
)

# Add executable
add_executable(gpkih
        ${GPKIH_SRC_DIR}/gpki.cpp
        # Agrega otros archivos fuente si es necesario
)

# Especifica los encabezados a incluir
target_include_directories(gpkih PUBLIC
        "${INCLUDE_DIR}"
        "${FMT_LIBRARY_INCLUDE}"
)

# Look for FMT
find_library(FMT_LIBRARY NAMES ${FMT_LIBRARY_NAMES} PATHS ${FMT_LIBRARY_PATH})

# Link libraries to gpkih
target_link_libraries(gpkih PRIVATE 
        gpkih_parse_lib 
        gpkih_db_lib 
        gpkih_actions_lib 
        gpkih_config_management_lib
        gpkih_help_lib 
        ${FMT_LIBRARY}
)

# Set install paths that will be used by CPack
# Note: make install is not intended to be run, this should be changed to use cpack components
#set(GPKIH_MISC_FILES
#        "${GPKIH_ROOT_DIR}/setup.sh"
#        "${GPKIH_ROOT_DIR}/COMANDOS.md"
#)
set(GPKIH_CONFIG_DIR
        "${GPKIH_ROOT_DIR}/config"
)
install(TARGETS gpkih DESTINATION bin)
install(DIRECTORY "${GPKIH_CONFIG_DIR}" DESTINATION .)
#install(FILES "${GPKIH_MISC_FILES}" DESTINATION .)

# Include CPack
# Note: CPack configuration must be done before including it
include(CPack)