cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_GMOCK ON)
set(INSTALL_GTEST OFF)


fetchcontent_declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
        EXCLUDE_FROM_ALL
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

fetchcontent_makeavailable(googletest)

if (NOT TARGET GTest::gtest)
    add_library(GTest::gtest ALIAS gtest)
    add_library(GTest::gtest_main ALIAS gtest_main)
    add_library(GTest::gmock ALIAS gmock)

endif ()


enable_testing()

add_executable(test_error_utils
        test_error_utils.cpp
)

target_link_libraries(test_error_utils
        cpp_error_utils
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

include(GoogleTest)


gtest_discover_tests(test_error_utils)
