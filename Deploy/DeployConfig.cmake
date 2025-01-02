# # Deploy/DeployConfig.cmake

# # Include the CPack module
# include(CPack)

# # Configuration for the deploy script
# set(COPY_LIBS_SCRIPT "${CMAKE_SOURCE_DIR}/Scripts/copyDependencies.sh")
# set(DEPENDENCIES_DIR "${CMAKE_BINARY_DIR}/DEPENDENCIES/")

# # Custom target to execute the dependency copy script
# add_custom_target(copy_dependencies ALL
#     COMMAND ${CMAKE_COMMAND} -E echo "Copying dependencies to ${DEPENDENCIES_DIR}"
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPENDENCIES_DIR}
#     COMMAND bash ${COPY_LIBS_SCRIPT} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CalFluxNeutron ${DEPENDENCIES_DIR}
#     DEPENDS CalFluxNeutron
# )


# # Install other files
# foreach(file ${OTHER_FILES})
#     install(FILES ${CMAKE_SOURCE_DIR}/${file} DESTINATION .)
# endforeach()
# # Other files to include in the installation
# set(OTHER_FILES
#   README.md
#   license-Qt.txt
# )

# # CPack configuration for creating DEB packages
# set(CPACK_GENERATOR "DEB")
# set(CPACK_PACKAGE_NAME "NeutronFluxCalculation")
# set(CPACK_PACKAGE_VERSION "1.0.0")
# set(CPACK_PACKAGE_CONTACT "andreiasimas8@gmail.com")
# set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andreia de Simas Vidalino")
# set(CPACK_PACKAGE_VENDOR "UERJ")
# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Package description (title, brief)")
# set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};${PROJECT_NAME}")

# # URL of the project
# set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/simasvidalino/Calc_fluxo_de_particulas_neutras")

# # Dependencies required for the DEB package
# set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.27), libglib2.0-0, zlib1g, libstdc++6, qt6-base-dev, qt6-base-plugins")
set(FOLDER_APPDIR ${CMAKE_SOURCE_DIR}/Deploy/${PROJECT_NAME}.AppDir)

set(PROJECT_NAME CalFluxNeutron)
set(EXECUTABLE_NAME ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME})
set(DESKTOP_FILE ${CMAKE_SOURCE_DIR}/Deploy/application/${PROJECT_NAME}.desktop)
set(ICON_FILE ${CMAKE_SOURCE_DIR}/Deploy/icons/${PROJECT_NAME}.png)
set(LINUXDEPLOY_PATH ${CMAKE_SOURCE_DIR}/Deploy/linuxdeploy-x86_64.AppImage)

# Verificar existência dos arquivos e caminhos necessários
if(NOT EXISTS ${LINUXDEPLOY_PATH})
    message(FATAL_ERROR "linuxdeploy-x86_64.AppImage not found: ${LINUXDEPLOY_PATH}")
else()
    message(STATUS "linuxdeploy found: ${LINUXDEPLOY_PATH}")
endif()

if(NOT EXISTS ${DESKTOP_FILE})
    message(FATAL_ERROR "Desktop file not found: ${DESKTOP_FILE}")
else()
    message(STATUS "Desktop file found: ${DESKTOP_FILE}")
endif()

if(NOT EXISTS ${ICON_FILE})
    message(FATAL_ERROR "Icon file not found: ${ICON_FILE}")
else()
    message(STATUS "Icon file found: ${ICON_FILE}")
endif()

if(NOT EXISTS ${EXECUTABLE_NAME})
    message(FATAL_ERROR "Executable not found: ${EXECUTABLE_NAME}")
else()
    message(STATUS "Executable found: ${EXECUTABLE_NAME}")
endif()

add_custom_target(prepare_AppDir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/share/applications
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FOLDER_APPDIR}/usr/share/icons/hicolor/256x256/apps

    COMMAND ${CMAKE_COMMAND} -E copy ${DESKTOP_FILE} ${FOLDER_APPDIR}/usr/share/applications/
    COMMAND ${CMAKE_COMMAND} -E copy ${ICON_FILE} ${FOLDER_APPDIR}/usr/share/icons/hicolor/256x256/apps/
    COMMAND ${CMAKE_COMMAND} -E copy ${EXECUTABLE_NAME} ${FOLDER_APPDIR}/usr/bin/
)

add_custom_target(deploy ALL
    COMMENT "Running linuxdeploy to create AppImage"

    COMMAND ${CMAKE_COMMAND} -E env "PATH=/home/andreia-simas/Qt/6.8.0/gcc_64/bin:$ENV{PATH}"
        ./Deploy/linuxdeploy-x86_64.AppImage
        --appdir=${FOLDER_APPDIR}
        --desktop-file=${DESKTOP_FILE}
        --icon-file=${ICON_FILE}
        --plugin qt
        --output appimage
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)


add_dependencies(deploy prepare_AppDir)
