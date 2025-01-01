# Deploy/DeployConfig.cmake

# Include the CPack module
include(CPack)

# Configuration for the deploy script
set(COPY_LIBS_SCRIPT "${CMAKE_SOURCE_DIR}/Scripts/copyDependencies.sh")
set(DEPENDENCIES_DIR "${CMAKE_BINARY_DIR}/DEPENDENCIES/")

# Custom target to execute the dependency copy script
add_custom_target(copy_dependencies ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Copying dependencies to ${DEPENDENCIES_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPENDENCIES_DIR}
    COMMAND bash ${COPY_LIBS_SCRIPT} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CalFluxNeutron ${DEPENDENCIES_DIR}
    DEPENDS CalFluxNeutron
)

# Other files to include in the installation
set(OTHER_FILES
  README.md
  license-Qt.txt
)

# Install other files
foreach(file ${OTHER_FILES})
    install(FILES ${CMAKE_SOURCE_DIR}/${file} DESTINATION .)
endforeach()

# CPack configuration for creating DEB packages
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "NeutronFluxCalculation")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "andreiasimas8@gmail.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andreia de Simas Vidalino")
set(CPACK_PACKAGE_VENDOR "UERJ")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Package description (title, brief)")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};${PROJECT_NAME}")

# URL of the project
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/simasvidalino/Calc_fluxo_de_particulas_neutras")

# Dependencies required for the DEB package
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.27), libglib2.0-0, zlib1g, libstdc++6, qt6-base-dev, qt6-base-plugins")

