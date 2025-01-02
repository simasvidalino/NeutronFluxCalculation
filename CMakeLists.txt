cmake_minimum_required(VERSION 3.10)

project(NeutronFluxCalculation VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(Utilities/Utilities)
add_subdirectory(DDMethod/DDMethod)
add_subdirectory(Interface/CalFluxNeutron)

message(STATUS "Project Source Directory: ${CMAKE_SOURCE_DIR}")

#Deploy flag
option(ENABLE_DEPLOY "Enable deploy process" OFF)

if(ENABLE_DEPLOY)
    message(STATUS "Deploy enabled.")
    include(Deploy/DeployConfig.cmake)
else()
    message(STATUS "Deploy disabled. Use -DENABLE_DEPLOY=ON to enable.")
endif()

