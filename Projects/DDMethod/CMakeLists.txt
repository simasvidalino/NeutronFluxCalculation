cmake_minimum_required(VERSION 3.5)

project(DDMethod LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Listt of headers
set(HEADERS
    DDNumericalMethold.h
    GausLegendreQuadrature.h
    LegendrePolynomial.h
    VariablesUsed.h
)

# List of cpps
set(SOURCES
    DDNumericalMethold.cpp
)

# Create a shared library (.so)
add_library(DDMethod SHARED ${HEADERS} ${SOURCES})

# Add includ directories
target_include_directories(DDMethod
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Install library
install(TARGETS DDMethod
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
