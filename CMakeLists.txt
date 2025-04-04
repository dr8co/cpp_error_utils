cmake_minimum_required(VERSION 3.20)
project(cpp_error_utils
        VERSION 0.1.0
        DESCRIPTION "C++ error handling utilities"
        HOMEPAGE_URL "https://github.com/dr8co/cpp_error_utils"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

# Set the build type to Release if it is not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

include(CMakeDependentOption)

cmake_dependent_option(CPP_ERR_BUILD_TESTING "Build tests" ON "PROJECT_IS_TOP_LEVEL" OFF)
cmake_dependent_option(CPP_ERR_BUILD_EXAMPLES "Build examples" ON "PROJECT_IS_TOP_LEVEL" OFF)
cmake_dependent_option(CPP_ERR_BUILD_DOC "Build documentation" OFF "PROJECT_IS_TOP_LEVEL" OFF)
cmake_dependent_option(CPP_ERR_PACKAGE "Package the library" OFF "PROJECT_IS_TOP_LEVEL" OFF)


# Set the path to additional CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(cpp_error_utils INTERFACE)

add_library(cpp_error_utils::cpp_error_utils ALIAS cpp_error_utils)

target_include_directories(cpp_error_utils INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(cpp_error_utils INTERFACE cxx_std_23)

if (CPP_ERR_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (CPP_ERR_PACKAGE)
    include(CEPackaging)
endif ()

if (CPP_ERR_BUILD_DOC)
    add_subdirectory(doc)
endif ()

if (CPP_ERR_BUILD_TESTING)
    add_subdirectory(tests)
endif ()

