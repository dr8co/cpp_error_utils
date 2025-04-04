# CEPackaging.cmake
# Configure packaging for the project

include(GNUInstallDirs)

install(TARGETS cpp_error_utils
        EXPORT cpp_error_utils
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install header files
install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
)

# Generate and install CMake config files
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/cpp_error_utils-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpp_error_utils-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cpp_error_utils-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpp_error_utils
)

install(EXPORT cpp_error_utils
        FILE cpp_error_utils-targets.cmake
        NAMESPACE cpp_error_utils::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpp_error_utils
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cpp_error_utils-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cpp_error_utils-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpp_error_utils
)

# Set package metadata
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "Ian Duncan")
set(CPACK_PACKAGE_CONTACT "Ian Duncan <dr8co@duck.com>")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")


set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/Packages")

set(CPACK_SOURCE_IGNORE_FILES
        /.git
        /.idea
        /.github
        /.vscode
        /.cache
        /build*
        /out*
        /cmake-build-*
        /CMakeFiles
        /CMakeScripts
        /CMakeModules
        /CMakeLists.txt.user
        /CMakeCache.txt
        /CTestTestfile.cmake
        /Makefile
        /Makefile.in
        /CPackConfig.cmake
        /CPackSourceConfig.cmake
        /CPackSourceConfig.cmake
        /CPack
)

# Specify package formats
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

# Set the package generator
if (APPLE)
    set(CPACK_GENERATOR "TXZ;DragNDrop")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CPACK_GENERATOR "TXZ;DEB;RPM")
else ()
    set(CPACK_GENERATOR "TXZ")
endif ()

# Set installation directories
set(CPACK_PACKAGE_INSTALL_DIRECTORY "cpp_error_utils")

# Include license and readme files
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_RPM_PACKAGE_LICENSE "MIT")

# Set the section of the package
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# Use the resource file for the license
set(CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE ON)

set(CPACK_PACKAGE_CHECKSUM "SHA256")

# Include CPack configuration
include(CPack)
