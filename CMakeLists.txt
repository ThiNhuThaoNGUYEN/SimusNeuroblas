# ============================================================================
# Require minimal version of cmake
# ============================================================================
cmake_minimum_required(VERSION 3.2.0)


# ============================================================================
# Set project name and languages
# ============================================================================
project(simuscale
        VERSION 1.0.0
        LANGUAGES CXX C)


# ============================================================================
# Get GNU standard installation directories (GNUInstallDirs module)
# ============================================================================
include(GNUInstallDirs)

# ============================================================================
# Tell CMake where to look for custom modules
# ============================================================================
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(GSL REQUIRED)

# ============================================================================
# Tell cmake where to build binary files.
# By GNU standards "executable programs that users can run" should go in
# bindir a.k.a ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
# and "executable programs to be run by other programs rather than by users"
# in libexecdir a.k.a ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}
# ============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})


# ============================================================================
# Set compilation flags (general and then build-type specific)
# ============================================================================
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -O0")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_C_FLAGS_DEBUG} -pg" CACHE STRING
    "Flags used for profiling"
    FORCE )
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_DEBUG} -pg" CACHE STRING
    "Flags used for profiling"
    FORCE )

mark_as_advanced(CMAKE_CXX_FLAGS_PROFILE
                 CMAKE_C_FLAGS_PROFILE)
# Update the documentation string of CMAKE_BUILD_TYPE for GUIs
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Profile."
    FORCE )


# ============================================================================
# Tell cmake about subdirectories to look into
# ============================================================================
add_subdirectory(src)


# ============================================================================
# Adds the 'dist' target (that will use CPack)
# ============================================================================
add_custom_target(dist COMMAND ${CMAKE_BUILD_TOOL} package_source)


# ============================================================================
# Add the 'uninstall' target (uses a custom script)
# ============================================================================
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_uninstall.cmake)




# ============================================================================
# Configure CPack
# ============================================================================
set(CPACK_PACKAGE_NAME "simuscale")
set(CPACK_PACKAGE_VENDOR "SiMuScale Development Team")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multi-scale simulation of populations of cells")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_AUTHORS "${CMAKE_SOURCE_DIR}/AUTHORS")
set(CPACK_RESOURCE_FILE_INSTALL "${CMAKE_SOURCE_DIR}/INSTALL")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
 "CMakeFiles"
 "Makefile"
 "_CPack_Packages"
 "CMakeCache.txt"
 ".*\\\\.svn"
 ".*\\\\.gz"
 ".*\\\\~"
 ".*\\\\.o"
 "/\\\\.git*"
 "build"
 ${CPACK_SOURCE_IGNORE_FILES}
)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
include(CPack)
