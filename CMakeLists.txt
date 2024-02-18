cmake_minimum_required(VERSION 3.10)

project(gpkih)

# Define los directorios fuente y de encabezado
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${SOURCE_DIR}")

# Agrega archivos fuente
add_executable(gpkih
        ${SOURCE_DIR}/gpki.cpp
        # Agrega otros archivos fuente si es necesario
)

# Especifica los encabezados a incluir
target_include_directories(gpkih PUBLIC
        ${INCLUDE_DIR}
)

set(FMT_LIBRARY_NAMES fmt)

if(WIN32)
        set(FMT_LIBRARY_PATH "C:/Program Files (x86)/FMT/lib")
        set(CMAKE_CXX_FLAGS "/O3")
else()
        set(FMT_LIBRARY_PATH "/usr/lib;/usr/local/lib")
        set(CMAKE_CXX_FLAGS "-O3")
endif()

# Look for FMT
find_library(FMT_LIBRARY NAMES ${FMT_LIBRARY_NAMES} PATHS ${FMT_LIBRARY_PATH})
target_link_libraries(gpkih PRIVATE ${FMT_LIBRARY})