cmake_minimum_required(VERSION 3.5)

project(Utilities LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DD_METHOD_BUILD /home/andreiasimas/Documentos/NeutronFluxCalculation/DDMethod/build)
set(DD_METHOD /home/andreiasimas/Documentos/NeutronFluxCalculation/DDMethod/DDMethod)

# Listt of headers
set(HEADERS
    DataMatrices.h
    ProjectStructs.h
)

# List of cpps
set(SOURCES
    #main.cpp
    DataMatrices.cpp
)

include_directories(${DD_METHOD}) #tbd
link_directories(${DD_METHOD})

# Create a shared library (.so)
add_library(Utilities SHARED ${HEADERS} ${SOURCES})

# Add includ directories
target_include_directories(Utilities
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Install library
install(TARGETS Utilities
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
