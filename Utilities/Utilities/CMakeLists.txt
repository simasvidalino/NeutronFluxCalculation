cmake_minimum_required(VERSION 3.5)

project(Utilities LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DD_METHOD "${CMAKE_SOURCE_DIR}/DDMethod/DDMethod")

# Listt of headers
set(HEADERS
    DataMatrices.h
    ProjectStructs.h
)

# List of cpps
set(SOURCES
    DataMatrices.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core)

include_directories(${DD_METHOD}) #tbd
link_directories(${DD_METHOD})

# Create a shared library (.so)
add_library(Utilities SHARED ${HEADERS} ${SOURCES})

target_link_libraries(Utilities
    PRIVATE Qt6::Core
)

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
