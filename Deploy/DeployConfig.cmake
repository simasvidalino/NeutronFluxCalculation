cmake_minimum_required(VERSION 3.14)
project(CalFluxNeutron_Deploy)

# ==============================================================================
# PROJECT CONFIGURATION
# ==============================================================================
set(PROJECT_NAME CalFluxNeutron)

# Qt Configuration Paths
# Ensure these paths point to the correct Qt installation for deployment.
set(QT_PATH "/home/andreia-simas/Qt/6.8.0/gcc_64")
set(QMAKE_PATH "${QT_PATH}/bin/qmake")

# Directory Definitions
set(FOLDER_APPDIR ${CMAKE_SOURCE_DIR}/Deploy/${PROJECT_NAME}.AppDir)
set(DEBUG_SYMBOLS_DIR ${CMAKE_SOURCE_DIR}/Deploy/DebugSymbols) # New folder for debug files
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)

# File Paths
set(EXECUTABLE_NAME ${BIN_DIR}/${PROJECT_NAME})
set(DESKTOP_FILE ${CMAKE_SOURCE_DIR}/Deploy/application/${PROJECT_NAME}.desktop)
set(ICON_FILE ${CMAKE_SOURCE_DIR}/Deploy/icons/${PROJECT_NAME}.png)
set(LINUXDEPLOY_PATH ${CMAKE_SOURCE_DIR}/Deploy/linuxdeploy-x86_64.AppImage)

# ==============================================================================
# TOOL DETECTION
# ==============================================================================

# Detect 'strip' and 'objcopy' tools.
if (CMAKE_STRIP)
    set(STRIP_TOOL ${CMAKE_STRIP})
elseif(UNIX)
    find_program(STRIP_TOOL strip)
endif()

if (NOT STRIP_TOOL)
    message(WARNING "Deploy: Strip tool NOT found. Binaries will retain debug symbols.")
endif()

find_program(OBJCOPY_TOOL objcopy)
if (NOT OBJCOPY_TOOL)
    message(WARNING "Deploy: objcopy tool NOT found. Debug symbols cannot be extracted separately.")
endif()

# Locate project-specific shared libraries
file(GLOB LIB_UTILITIES "${LIB_DIR}/libUtilities.so*")
file(GLOB LIB_DDMETHOD "${LIB_DIR}/libDDMethod.so*")

# ==============================================================================
# TARGET: prepare_AppDir
# Description: Creates AppDir, extracts debug symbols to a separate folder, and strips binaries.
# ==============================================================================
add_custom_target(prepare_AppDir ALL
    # 1. Clean and recreate directories
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${FOLDER_APPDIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEBUG_SYMBOLS_DIR}

    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/share/applications
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/share/icons/hicolor/256x256/apps
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DEBUG_SYMBOLS_DIR}

    # 2. Copy assets
    COMMAND ${CMAKE_COMMAND} -E echo "Deploy: Copying assets..."
    COMMAND ${CMAKE_COMMAND} -E copy ${DESKTOP_FILE} ${FOLDER_APPDIR}/usr/share/applications/
    COMMAND ${CMAKE_COMMAND} -E copy ${ICON_FILE} ${FOLDER_APPDIR}/usr/share/icons/hicolor/256x256/apps/

    # 3. Copy binaries (Full size with debug symbols)
    COMMAND ${CMAKE_COMMAND} -E echo "Deploy: Copying binaries..."
    COMMAND ${CMAKE_COMMAND} -E copy ${EXECUTABLE_NAME} ${FOLDER_APPDIR}/usr/bin/
    COMMAND ${CMAKE_COMMAND} -E copy ${LIB_UTILITIES} ${FOLDER_APPDIR}/usr/lib/
    COMMAND ${CMAKE_COMMAND} -E copy ${LIB_DDMETHOD} ${FOLDER_APPDIR}/usr/lib/

    # 4. Extract Debug Symbols and Strip
    COMMAND ${CMAKE_COMMAND} -E echo "Deploy: Extracting debug symbols to ${DEBUG_SYMBOLS_DIR}..."

    # --- Process Main Executable ---
    # Extract symbols to external folder
    COMMAND ${OBJCOPY_TOOL} --only-keep-debug ${FOLDER_APPDIR}/usr/bin/${PROJECT_NAME} ${DEBUG_SYMBOLS_DIR}/${PROJECT_NAME}.debug
    # Strip the binary inside AppDir
    COMMAND ${STRIP_TOOL} --strip-all ${FOLDER_APPDIR}/usr/bin/${PROJECT_NAME}
    # Link the stripped binary to the external debug file (optional, helps gdb find it if in same folder)
    COMMAND ${OBJCOPY_TOOL} --add-gnu-debuglink=${DEBUG_SYMBOLS_DIR}/${PROJECT_NAME}.debug ${FOLDER_APPDIR}/usr/bin/${PROJECT_NAME}

    # --- Process Libraries (Optional: usually only needed if you debug libs heavily) ---
    # Strip libraries inside AppDir
    COMMAND ${STRIP_TOOL} --strip-unneeded ${FOLDER_APPDIR}/usr/lib/libUtilities.so* || echo "Warning: Failed to strip libUtilities."
    COMMAND ${STRIP_TOOL} --strip-unneeded ${FOLDER_APPDIR}/usr/lib/libDDMethod.so* || echo "Warning: Failed to strip libDDMethod."

    COMMENT "Deploy: Preparing AppDir and separating debug symbols..."
)

# Ensure the main application is built before attempting to deploy
if(TARGET CalFluxNeutron)
    add_dependencies(prepare_AppDir CalFluxNeutron)
endif()

# ==============================================================================
# TARGET: deploy
# Description: Generates the final AppImage using linuxdeploy.
# ==============================================================================
add_custom_target(deploy ALL
    COMMAND ${CMAKE_COMMAND} -E env
        "PATH=${QT_PATH}/bin:$ENV{PATH}"
        "QMAKE=${QMAKE_PATH}"
        "LD_LIBRARY_PATH=${QT_PATH}/lib:$ENV{LD_LIBRARY_PATH}"
        ${LINUXDEPLOY_PATH}
        --appdir=${FOLDER_APPDIR}
        --desktop-file=${DESKTOP_FILE}
        --icon-file=${ICON_FILE}
        --executable=${FOLDER_APPDIR}/usr/bin/${PROJECT_NAME}
        --plugin qt
        --output appimage
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Deploy: Generating AppImage..."
)

# Ensure AppDir is ready before running linuxdeploy
add_dependencies(deploy prepare_AppDir)
