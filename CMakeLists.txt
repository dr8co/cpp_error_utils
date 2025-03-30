cmake_minimum_required(VERSION 3.30)
project(cpp_error_utils)

set(CMAKE_CXX_STANDARD 23)

add_library(cpp_error_utils INTERFACE)

target_include_directories(cpp_error_utils INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(cpp_error_utils INTERFACE cxx_std_23)

add_subdirectory(examples)
