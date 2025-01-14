cmake_minimum_required(VERSION 3.5)

project(CalFluxNeutron VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DD_METHOD_DIR "${CMAKE_SOURCE_DIR}/DDMethod/DDMethod")
set(UTILITIES_DIR "${CMAKE_SOURCE_DIR}/Utilities/Utilities")
set(LIBS "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Core)

include_directories(${Qt6Widgets_INCLUDE_DIRS} ${Qt6Core_INCLUDE_DIRS} ${Qt6Charts_INCLUDE_DIRS})
include_directories(${DD_METHOD_DIR})
include_directories(${UTILITIES_DIR})
include_directories(${LIBS})
include_directories(".")

set(PROJECT_SOURCES
    main.cpp
    AbsorptionRateWidget.cpp
    AbsorptionRateWidget.h
    AbsorptionRateWidget.ui
    ChartView.cpp
    ChartView.h
    CrossSectionFileDlg.cpp
    CrossSectionFileDlg.h
    CrossSectionFileDlg.ui
    CustomComboBox.h
    CustomComboBox.cpp
    CustomTableModel.cpp
    CustomTableModel.h
    InterFaceDefinitions.cpp
    InterFaceDefinitions.h
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    MapRegion.cpp
    MapRegion.h
    MapRegion.ui
    NeutronAnalysisChartWidget.h
    NeutronAnalysisChartWidget.cpp
    NeutronAnalysisChartWidget.ui
    NeutronFlowJsonIO.cpp
    NeutronFlowJsonIO.h
    NJOYInputDlg.cpp
    NJOYInputDlg.h
    NJOYInputDlg.ui
    ParseFile.cpp
    ParseFile.h
    RegionInputData.cpp
    RegionInputData.h
    RegionInputData.ui
    TableInputDlg.cpp
    TableInputDlg.h
    TableInputDlg.ui
    Worker.cpp
    Worker.h
)

qt_add_resources(PROJECT_SOURCES Resources.qrc)

if(Qt${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CalFluxNeutron
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(CalFluxNeutron SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(CalFluxNeutron
            ${PROJECT_SOURCES}
        )
    endif()
endif()

add_dependencies(CalFluxNeutron DDMethod Utilities)

target_link_libraries(CalFluxNeutron PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Core ${LIBS}/libDDMethod.so ${LIBS}/libUtilities.so)

set_target_properties(CalFluxNeutron PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS CalFluxNeutron
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(Qt${QT_VERSION_MAJOR} EQUAL 6)
    qt_finalize_executable(CalFluxNeutron)
endif()

qt_finalize_executable(CalFluxNeutron)

