cmake_minimum_required(VERSION 3.20)

add_executable(example
        main.cpp
)

target_compile_options(example PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wpedantic
)

target_link_libraries(example
        cpp_error_utils
)