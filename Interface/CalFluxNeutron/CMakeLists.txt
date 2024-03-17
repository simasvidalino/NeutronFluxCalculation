cmake_minimum_required(VERSION 3.5)

project(CalFluxNeutron VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DD_METHOD "${CMAKE_SOURCE_DIR}/DDMethod/DDMethod")
set(UTILITIES "${CMAKE_SOURCE_DIR}/Utilities/Utilities")
set(LIBS "${CMAKE_SOURCE_DIR}/build/lib/")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts)

include_directories(./)
include_directories(${DD_METHOD}) #tbd
include_directories(${LIBS}) #tbd
include_directories(${UTILITIES}) #tbd

set(PROJECT_SOURCES
    main.cpp
    ChartView.cpp
    ChartView.h
    CrossSectionFileDlg.cpp
    CrossSectionFileDlg.h
    CrossSectionFileDlg.ui
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

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CalFluxNeutron
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET CalFluxNeutron APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(CalFluxNeutron SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(CalFluxNeutron
            ${PROJECT_SOURCES}
        )
    endif()
endif()

link_directories(${DD_METHOD})

target_link_libraries(CalFluxNeutron PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts
    ${LIBS}/libDDMethod.so  ${LIBS}/libUtilities.so)

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

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CalFluxNeutron)
endif()


