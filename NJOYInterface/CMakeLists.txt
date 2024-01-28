cmake_minimum_required(VERSION 3.5)

project(TCC VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Pdf)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS PdfWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS PrintSupport)
find_package(Qt6 REQUIRED COMPONENTS Svg)
find_package(Qt6 REQUIRED COMPONENTS SvgWidgets)

set(PROJECT_SOURCES
        NJOYGeneralParametersWidget.cpp
        NJOYGeneralParametersWidget.h
        NJOYGeneralParametersWidget.ui
        main.cpp
        MainWindow.cpp
        MainWindow.h
        NJOYBroadrInputDlg.cpp
        NJOYBroadrInputDlg.h
        NJOYBroadrInputDlg.ui
        MainWindow.ui
        NJOYChangeNJOYOutputFormat.cpp
        NJOYChangeNJOYOutputFormat.h
        NJOYChoosingIsotopesWidget.cpp
        NJOYChoosingIsotopesWidget.h
        NJOYChoosingIsotopesWidget.ui
        NJOYChoosingModulesJsonIO.cpp
        NJOYChoosingModulesJsonIO.h
        NJOYChoosingModulesWidget.cpp
        NJOYChoosingModulesWidget.h
        NJOYChoosingModulesWidget.ui
        NJOYCrossSectionPDFReader.cpp
        NJOYCrossSectionPDFReader.h
        NJOYEnterDataWizard.cpp
        NJOYEnterDataWizard.h
        NJOYGaminrInputDlg.cpp
        NJOYGaminrInputDlg.h
        NJOYGaminrInputDlg.ui
        NJOYGrouprInputDlg.cpp
        NJOYGrouprInputDlg.h
        NJOYGrouprInputDlg.ui
        NJOYGroupInputweightFunctionOptions.cpp
        NJOYGroupInputweightFunctionOptions.h
        NJOYGroupInputweightFunctionOptions.ui
        NJOYInterfaceDefinitions.h
        NJOYInputTableWidget.h
        NJOYInputTableWidget.cpp
        NJOYInputTableWidget.ui
        NJOYMacroCrossSectionCalculator.cpp
        NJOYMacroCrossSectionCalculator.h
        NJOYModerInputDlg.cpp
        NJOYModerInputDlg.h
        NJOYModerInputDlg.ui
        NJOYModulesBase.cpp
        NJOYModulesBase.h
        NJOYProjectJsonIO.cpp
        NJOYProjectJsonIO.h
        NJOYPDFPrinter.cpp
        NJOYPDFPrinter.h
        NJOYPDFViewDlg.cpp
        NJOYPDFViewDlg.h
        NJOYPDFViewWidget.cpp
        NJOYPDFViewWidget.h
        NJOYPDFViewWidget.ui
        NJOYReconrInputDlg.cpp
        NJOYReconrInputDlg.h
        NJOYReconrInputDlg.ui
        NJoySplashScreen.cpp
        NJoySplashScreen.h
        NJoySplashScreen.ui
        NJOYTooltipsHeader.h
        NJOYWizard.h
        NJOYWizard.cpp
        NJOYWizardPageBase.cpp
        NJOYWizardPageBase.h
        NJOYWrapper.cpp
        NJOYWizard.h
        NJOYStackWidget.cpp
        NJOYStackWidget.h
        NJOYStackWidget.ui
        NJOYUnresrInputDlg.cpp
        NJOYUnresrInputDlg.h
        NJOYUnresrInputDlg.ui
)

qt_add_resources(PROJECT_SOURCES Resources.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TCC
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TCC APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TCC SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TCC
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TCC PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(TCC PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(TCC PRIVATE Qt${QT_VERSION_MAJOR}::Pdf)
target_link_libraries(TCC PRIVATE Qt${QT_VERSION_MAJOR}::PdfWidgets)
target_link_libraries(TCC PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport)
target_link_libraries(TCC PRIVATE Qt6::Svg)
target_link_libraries(TCC PRIVATE Qt6::SvgWidgets)


set(SRC_LIST
    ${SRC_LIST}
    ${CMAKE_CURRENT_SOURCE_DIR}/a.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/b.cpp
    PARENT_SCOPE
)


set_target_properties(TCC PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS TCC
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TCC)
endif()
