cmake_minimum_required(VERSION 3.10)

project(gpkih)

# Define los directorios fuente y de encabezado
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
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

# Busca dependencias externas
find_library(FMT_LIBRARY NAMES fmt PATHS /usr/local/lib /usr/lib)
target_link_libraries(gpkih PRIVATE ${FMT_LIBRARY})

# Compila con optimizaciones
set(CMAKE_CXX_FLAGS "-O3")
