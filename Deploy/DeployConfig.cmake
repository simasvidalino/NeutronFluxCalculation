cmake_minimum_required(VERSION 3.14)
project(CalFluxNeutron_Deploy)

# --- PATH CONFIGURATIONS ---
# Adjust if necessary, but it seems correct based on your environment
set(QT_PATH "/home/andreia-simas/Qt/6.8.0/gcc_64")
set(QMAKE_PATH "${QT_PATH}/bin/qmake")

# Project Paths
set(FOLDER_APPDIR ${CMAKE_SOURCE_DIR}/Deploy/${PROJECT_NAME}.AppDir)
set(PROJECT_NAME CalFluxNeutron)

# Where compiled binaries are located (adjust if your build folder is different)
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(LIB_DIR ${CMAKE_BINARY_DIR}/lib) # Usually libs are here or in bin

set(EXECUTABLE_NAME ${BIN_DIR}/${PROJECT_NAME})
set(DESKTOP_FILE ${CMAKE_SOURCE_DIR}/Deploy/application/${PROJECT_NAME}.desktop)
set(ICON_FILE ${CMAKE_SOURCE_DIR}/Deploy/icons/${PROJECT_NAME}.png)
set(LINUXDEPLOY_PATH ${CMAKE_SOURCE_DIR}/Deploy/linuxdeploy-x86_64.AppImage)

# --- CHECKS ---
if(NOT EXISTS ${LINUXDEPLOY_PATH})
    message(FATAL_ERROR "linuxdeploy-x86_64.AppImage not found at: ${LINUXDEPLOY_PATH}")
endif()

if(NOT EXISTS ${EXECUTABLE_NAME})
    message(FATAL_ERROR "Executable not found at: ${EXECUTABLE_NAME}. Did you build the project first?")
endif()

# Try to find custom libraries to copy as well
# If names are different (e.g., libUtilities.so.1), adjust here
file(GLOB LIB_UTILITIES "${LIB_DIR}/libUtilities.so*")
file(GLOB LIB_DDMETHOD "${LIB_DIR}/libDDMethod.so*")

# --- APPDIR PREPARATION ---
add_custom_target(prepare_AppDir ALL
    # 1. Clean and create directories
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${FOLDER_APPDIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/share/applications
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/share/icons/hicolor/256x256/apps

    # 2. Copy Executable and Support Files
    COMMAND ${CMAKE_COMMAND} -E copy ${DESKTOP_FILE} ${FOLDER_APPDIR}/usr/share/applications/
    COMMAND ${CMAKE_COMMAND} -E copy ${ICON_FILE} ${FOLDER_APPDIR}/usr/share/icons/hicolor/256x256/apps/
    COMMAND ${CMAKE_COMMAND} -E copy ${EXECUTABLE_NAME} ${FOLDER_APPDIR}/usr/bin/

    # 3. COPY YOUR LIBS (Crucial to avoid mixing with system libs)
    # If variables are empty, it won't copy and will warn in log, but won't fail the build
    COMMAND ${CMAKE_COMMAND} -E copy ${LIB_UTILITIES} ${FOLDER_APPDIR}/usr/lib/ || echo "Warning: libUtilities not found to copy"
    COMMAND ${CMAKE_COMMAND} -E copy ${LIB_DDMETHOD} ${FOLDER_APPDIR}/usr/lib/ || echo "Warning: libDDMethod not found to copy"

    COMMENT "Preparing AppDir folder structure..."
)

# --- APPIMAGE GENERATION ---
add_custom_target(deploy ALL
    COMMENT "Running linuxdeploy with forced Qt 6.8.0..."

    # Define environment EXACTLY for Qt 6.8.0
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=${QT_PATH}/bin:$ENV{PATH}"
        "QMAKE=${QMAKE_PATH}"
        "LD_LIBRARY_PATH=${QT_PATH}/lib:$ENV{LD_LIBRARY_PATH}"

        # LinuxDeploy Command
        ${LINUXDEPLOY_PATH}
        --appdir=${FOLDER_APPDIR}
        --desktop-file=${DESKTOP_FILE}
        --icon-file=${ICON_FILE}
        --executable=${FOLDER_APPDIR}/usr/bin/${PROJECT_NAME}
        --plugin qt
        --output appimage

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Ensures preparation runs before deploy
add_dependencies(deploy prepare_AppDir)
