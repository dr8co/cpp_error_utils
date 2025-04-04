# cpp_error_utils

[![CMake Build and Test](https://github.com/dr8co/cpp_error_utils/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=main)](https://github.com/dr8co/cpp_error_utils/actions/workflows/cmake-multi-platform.yml)

A modern C++23 library providing robust error handling utilities
and abstractions built around `std::expected` and `std::error_code`.

This library provides tools to simplify error handling, improve code readability,
and ensure robust error management.

## Features

- Error handling with context using `Result<T>` type (based on `std::expected`).
- System error code integration with `std::error_code` and `std::error_condition`.
- Exception handling utilities with automatic conversion to error codes.
- Convenient error handling for C system calls and errno.
- Custom error categories and conditions.
- Support for chaining error operations.
- Zero overhead abstractions.
- Support for exception-safe code.
- Monadic operations for functional composition on `Result<T>`, from `std::expected`.
- Lightweight and easy to integrate into existing projects.

## Thread Safety

This library is designed to be thread-safe.

However, the thread safety of your code depends on how you use the library.

If you share `Result<T>` objects across threads, ensure proper synchronization.

## Requirements

- C++23 compatible compiler (GCC 14.2.0+ or Clang 19+) with a supported implementation
  of the C++ standard library, such as libstdc++ 14.2.0 (bundled with GCC C++ compiler).

The library currently supports only UNIX-like systems (Linux, macOS, etc.).
Windows support is not guaranteed but will be added in the future.

## Installation

The library is header-only, so you can easily include it in your project.

### Option 1: Copy the header files (the simplest method)

1. Download the header files from the [include/](./include/ "includes") directory of this repository.
2. Place them in your project's include directory.
3. Include the headers in your source files as needed.

### Option 2: Use CMake FetchContent module (recommended for projects already using CMake)

If your project uses CMake, you can use the FetchContent module to include this library as a dependency.

In your `CMakeLists.txt`, add the following lines:

```cmake
include(FetchContent)

FetchContent_Declare(
    cpp_error_utils
    GIT_REPOSITORY https://github.com/dr8co/cpp_error_utils.git
    GIT_TAG        main
)

FetchContent_MakeAvailable(cpp_error_utils)

target_link_libraries(your_target_name PRIVATE cpp_error_utils::cpp_error_utils)
```

### Option 3: System-wide installation

Prebuilt binaries are available at the
[release page](https://github.com/dr8co/cpp_error_utils/releases "click to check out releases").

You can download the latest release and install it with your package manager.

If you want to manually build and install the library system-wide,
you can do so by following these steps:

1. Clone the repository:

    ```bash
    git clone https://github.com/dr8co/cpp_error_utils.git
    ```

2. Navigate into the cloned directory:

    ```bash
    cd cpp_error_utils
    ```

3. Build and install the library:

    ```bash
    mkdir build && cd build
    cmake ..
    cmake --build . --config Release
    cmake --install . --config Release # might need sudo
    ```

    The following CMake options are available:

    - `CPP_ERR_BUILD_EXAMPLES` - Build example programs (ON by default)
    - `CPP_ERR_BUILD_TESTING` - Build tests (ON by default)
    - `CPP_ERR_BUILD_DOC` - Build documentation (OFF by default)
    - `CPP_ERR_PACKAGE` - Create installation package (OFF by default)

## Usage Examples

Include the necessary headers in your C++ files:

```cpp
#include "error_utils.hpp"

// or, depending on your installation method:

#include <error_utils.hpp>
```

### Basic Result Type Usage

```cpp
#include <error_utils.hpp>
#include <string_view>
#include <print>
#include <filesystem>

namespace fs = std::filesystem;

Result<void> print_file_details(std::string_view file_name) {
    std::error_code ec{};
    const fs::path file_path{file_name};

    const auto status = fs::status(file_path, ec);

    if (ec) {
        // error_utils::Error can be constructed with std::error_code directly
        return std::unexpected(error_utils::Error{
            ec, std::format("Failed to determine the attributes of '{}'", file_name)
        });
    }

    if (!fs::exists(status)) {
        // std::errc constants can also be used, as well as error_utils::ExtraError
        return std::unexpected(error_utils::Error{
            std::errc::no_such_file_or_directory, std::format("File '{}' does not exist.", file_name)
        });
    }

    std::print("{}", fs::absolute(file_path).string());

    switch (status.type()) {
        case fs::file_type::regular:
            std::println(" is a regular file.");
            break;
        case fs::file_type::directory:
            std::println(" is a directory.");
            break;
        case fs::file_type::fifo:
            std::println(" is a named IPC pipe.");
            break;
        case fs::file_type::block:
            std::println(" is a block device.");
            break;
        case fs::file_type::socket:
            std::println(" is a named IPC socket.");
            break;
        case fs::file_type::symlink:
            std::println(" is a symbolic link.");
            break;
        case fs::file_type::character:
            std::println(" is a character device.");
            break;
        case fs::file_type::unknown:
            std::println(" has 'unknown' type.");
            break;
        case fs::file_type::none:
            std::println(" has 'not-evaluated-yet' type");
            break;
        default:
            std::println(" has an implementation-defined type.");
    }

    std::print("File permissions: ");
    const auto file_perms = status.permissions();

    // Adapted from https://en.cppreference.com/w/cpp/filesystem/file_status/permissions
    auto show = [=](const char op, const fs::perms perm) {
        std::print("{}", fs::perms::none == (perm & file_perms) ? '-' : op);
    };

    show('r', fs::perms::owner_read);
    show('w', fs::perms::owner_write);
    show('x', fs::perms::owner_exec);
    show('r', fs::perms::group_read);
    show('w', fs::perms::group_write);
    show('x', fs::perms::group_exec);
    show('r', fs::perms::others_read);
    show('w', fs::perms::others_write);
    show('x', fs::perms::others_exec);

    std::println();

    return {}; // Success value for Result<void>
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::println("Usage: {} <file>", argv[0]);
        return 1;
    }

    // monadic operations
    auto res = print_file_details(argv[1])
        .or_else(
            [](const error_utils::Error &err) -> Result<void> {
                std::println(stderr, "An error occurred: {}", err.message());
                return {};
            }
        );
    return 0;
}
```

### Exception Handling

```cpp
// Convert exceptions to Result<T>
#include <error_utils.hpp>

int main() {
    auto result = error_utils::try_catch([]() -> error_utils::IntResult {
        // Your code that might throw exceptions
        return 42;
    });

    if (result) {
      std::println("Success: {}", result.value());
    } else {
        std::println("Error: {}", result.error().message());
    }

    return 0;
}
```

### System Call Error Handling

```cpp
// Handle errno-based C API calls
IntResult create_directory(const std::string& path) {
    return error_utils::invoke_with_syscall_api(
        [&] noexcept { return mkdir(path.c_str(), 0755); },
        std::format("Failed to create directory '{}'", path)
    );
}
```

Check out [more examples](./examples/main.cpp "examples") for additional usage patterns.

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Submit a pull request with a detailed description of your changes.

## License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE "MIT license")
file for details.

## Contact

For questions or feedback, feel free to open an issue or contact me directly at
[dr8co@duck.com](mailto:dr8co@duck.com "Ian Duncan").
