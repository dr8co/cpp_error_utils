#include <error_utils.hpp>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <print>
#include <unistd.h>
#include <sys/stat.h>

// Example: Wrapper for C file API
StringResult read_file_c_api(const std::string &filename) {
    // Open a file using C API
    errno = 0;
    const int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        return error_utils::make_error_from_errno<std::string>(std::format("Opening '{}'", filename));
    }

    // Read file content
    std::string content;
    char buffer[4096]{};
    ssize_t bytes_read = 0;

    while (true) {
        errno = 0;
        bytes_read = read(fd, buffer, sizeof(buffer));

        if (bytes_read == 0) {
            // End of the file
            break;
        }
        if (bytes_read == -1) {
            // Error occurred
            const auto err = error_utils::last_error();
            close(fd);
            return error_utils::make_error<std::string>(err, std::format("Reading '{}'", filename));
        }

        content.append(buffer, bytes_read);
    }

    close(fd);
    return content;
}

// Example: C++ stream with error handling
StringResult read_file_cpp_api(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // For C++ streams, we need to map to appropriate errc
        return error_utils::make_error<std::string>(std::errc::no_such_file_or_directory,
                                                    std::format("Opening '{}'", filename));
    }

    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + '\n';
    }

    if (file.bad()) {
        return error_utils::make_error<std::string>(std::errc::io_error,
                                                    std::format("Reading '{}'", filename));
    }

    return content;
}

// Example: Custom error checking logic
IntResult parse_positive_number(const std::string &str) {
    try {
        std::size_t pos = 0;
        int value = std::stoi(str, &pos);

        // Check if the entire string was consumed
        if (pos != str.length()) {
            return error_utils::make_error<int>(std::errc::invalid_argument,
                                                "String contains non-numeric characters");
        }

        if (value < 0) {
            return error_utils::make_error<int>(std::errc::invalid_argument,
                                                "Number must be positive");
        }

        return value;
    } catch (const std::invalid_argument &) {
        return error_utils::make_error<int>(std::errc::invalid_argument,
                                            "Invalid number format");
    } catch (const std::out_of_range &) {
        return error_utils::make_error<int>(std::errc::result_out_of_range,
                                            "Number out of range");
    }
}

// Using try_catch for exception conversion
IntResult parse_number(const std::string &str) {
    return error_utils::try_catch([&str] {
        return std::stoi(str);
    }, std::format("Failed to parse '{}'", str));
}

// Example: Chaining error calls
Result<int> get_file_size(const std::string &filename) {
    auto content_result = read_file_cpp_api(filename);
    if (!content_result) {
        // Forward the error
        return std::unexpected(content_result.error());
    }

    return static_cast<int>(content_result.value().size());
}

// Example of a function that returns a void or an error
VoidResult create_directory(const std::string_view path) {
    // Simulate directory creation with a hardcoded check
    if (path.empty()) {
        return error_utils::make_error<void>(std::errc::invalid_argument, "Empty path");
    }

    if (path == "/root/secured") {
        return error_utils::make_error<void>(std::errc::permission_denied,
                                             std::format("Failed to create directory '{}'", path));
    }

    // Directory creation successful
    return {}; // Success case for void
}

// Using invoke_with_syscall_api() for simpler C API error handling
IntResult create_directory_alt(const std::string &path) {
    return error_utils::invoke_with_syscall_api(
        [&] noexcept { return mkdir(path.c_str(), 0755); },
        std::format("Failed to create directory '{}'", path)
    );
}

// Example of chaining expected results
Result<std::vector<int>> read_numbers_from_file(const std::string &filename) {
    // Read the file
    auto content_result = read_file_cpp_api(filename);
    if (!content_result) {
        // Forward the error
        return std::unexpected(content_result.error());
    }

    // Split content into lines
    std::string content = content_result.value();
    std::vector<std::string> lines;
    size_t pos = 0;
    while ((pos = content.find('\n')) != std::string::npos) {
        if (std::string line = content.substr(0, pos); !line.empty()) {
            lines.push_back(line);
        }
        content.erase(0, pos + 1);
    }

    // Parse each line as an integer
    std::vector<int> numbers;
    for (const auto &line : lines) {
        auto num_result = parse_number(line);
        if (!num_result) {
            return error_utils::make_error<std::vector<int>>(
                num_result.error().error_code(),
                std::format("Line '{}': {}", line, num_result.error().message())
            );
        }
        numbers.push_back(num_result.value());
    }

    return numbers;
}

// Example functions that might throw exceptions
std::vector<int> create_vector_of_size(const size_t size) {
    if (size > 1'000'000'000) {
        throw std::length_error("Vector size too large");
    }
    return std::vector<int>(size);
}

int divide(const int a, const int b) {
    if (b == 0) {
        throw std::invalid_argument("Division by zero");
    }
    return a / b;
}

std::optional<int> get_value() {
    constexpr std::optional<int> opt;
    // Intentionally not initialized
    return opt;
}

// Example usage with C++23 format and try_catch wrapper
void example_with_try_catch() {
    // Example 1: Catching length_error
    std::println("Allocating a large vector...");
    auto res1 = error_utils::try_catch([] {
        return create_vector_of_size(2'000'000'000);
    });

    if (!res1) {
        auto err = res1.error().error_code();
        std::println("\tError occurred: {} (code: {}).", res1.error().message(), err.value());

        // Check against a specific error
        if (err == ExtraError::length_error) {
            std::println("\tSpecifically a length_error exception.");
        }

        // Check against error condition
        if (err == ExtraErrorCondition::logic_error) {
            std::println("\tThis is categorized as a logic error.");
        }
    }
    // Example 2: Catching invalid_argument
    std::println("Dividing by zero...");
    auto res2 = error_utils::try_catch([] {
        return divide(10, 0);
    });

    if (!res2) {
        auto err = res2.error().error_code();
        std::println("\tError occurred: {} (code: {})", res2.error().message(), err.value());

        // Check against error condition
        if (err == ExtraErrorCondition::logic_error) {
            std::println("\tThis is also categorized as a logic error.");
        }
    }

    // Example 3: Catching bad_optional_access
    std::println("Accessing an uninitialized optional...");
    auto res3 = error_utils::try_catch([] {
        const auto opt = get_value();
        return opt.value(); // Will throw
    });

    if (!res3) {
        auto err = res3.error().error_code();
        std::println("\tError occurred: {} (code: {})", res3.error().message(), err.value());

        // Check against error condition
        if (err == ExtraErrorCondition::access_error) {
            std::println("\tThis is categorized as an access error.");
        }
    }
}

// Function that returns error_code directly
std::error_code allocate_large_buffer(const size_t size) {
    try {
        if (size > 1'000'000'000) {
            throw std::length_error("Buffer size too large");
        }

        std::vector<int> buffer(size);
        return {}; // Success
    } catch (const std::exception &) {
        return ExtraError::exception;
    } catch (...) {
        return ExtraError::unknown_exception;
    }
}

// Using first_of to try multiple alternatives
StringResult read_config_file() {
    return error_utils::first_of({
        read_file_c_api("config.ini"),
        read_file_c_api("/etc/myapp/config.ini"),
        read_file_c_api("/usr/local/etc/myapp/config.ini")
    });
}

// Using is_any_of to check multiple error conditions
bool is_file_access_error(const error_utils::Error &error) {
    return error.is_any_of(
        std::errc::permission_denied,
        std::errc::no_such_file_or_directory,
        std::errc::file_exists,
        ExtraError::invalid_argument,
        make_error_code(ExtraError::invalid_argument),
        ExtraErrorCondition::access_error,
        make_error_condition(ExtraErrorCondition::other_error)
    );
}

int main() {
    // Example 1: Reading with C API
    std::println("=== Example 1: Reading with C API ===");
    if (auto c_result = read_file_c_api("/etc/passwd")) {
        std::println("File size: {} bytes.", c_result.value().size());
    } else {
        std::println("Error: ", c_result.error().message());
    }

    // Example 2: Reading with C++ API
    std::println("\n=== Example 2: Reading with C++ API ===");
    if (auto cpp_result = read_file_cpp_api("nonexistent.txt"); !cpp_result) {
        std::println("Error: {}.", cpp_result.error().message());

        // Check specific error
        if (cpp_result.error().is(std::errc::no_such_file_or_directory)) {
            std::println("The file doesn't exist.");
        }

        if (is_file_access_error(cpp_result.error())) {
            std::println("This is a file access error.");
        }
    }

    // Example 3: Using with custom logic
    std::println("\n=== Example 3: Using with custom logic ===");
    std::println("Parsing -42:");
    if (auto num_result = parse_positive_number("-42"); !num_result) {
        std::println("\tParse error: {}.", num_result.error().message());
    }

    std::println("Parsing 123abc:");

    // Monadic operations
    auto int_result = parse_positive_number("123abc")
                      // Success path
                      .and_then([](int res) -> VoidResult {
                          std::println("\tParsed number: {}", res);
                          return {}; // Success
                      })
                      // Failure path
                      .or_else([](const error_utils::Error &err) -> VoidResult {
                          std::println("\tError: {}.", err.message());

                          // Check for a specific error type
                          if (err.is(std::errc::invalid_argument)) {
                              std::println("\tThis was an invalid argument error!");
                          }

                          return {};
                      });

    // Example 4: Using try_catch wrapper
    std::println("\n=== Example 4: Using try_catch wrapper ===");
    if (auto num = parse_number("xxxx")) {
        std::println("Parsed number: {}", num.value());
    } else {
        std::println("Error: {}.", num.error().message());
    }

    // Example 5: Function that returns void or error
    std::println("\n=== Example 5: Function that returns void or error ===");
    auto dir_result = create_directory("/root/secured")
                      .and_then([]-> VoidResult {
                          std::println("Directory created successfully");
                          return {};
                      })
                      .or_else([](const error_utils::Error &error) -> Result<void> {
                          std::println("Error: {}.", error.message());
                          return {};
                      });

    // Example 6: Using invoke_with_syscall_api() for C API
    std::println("\n=== Example 6: Using invoke_with_syscall_api() for C API ===");
    if (auto dir_result_alt = create_directory_alt("/root/secured")) {
        std::println("Directory created successfully");
    } else {
        std::println("Error: {}.", dir_result_alt.error().message());
    }

    // Example 7: Error chaining
    std::println("\n=== Example 7: Error chaining ===");
    if (auto size_result = get_file_size("sample.txt"); !size_result) {
        std::println("Failed to get size: {}.", size_result.error().message());
    }

    // Example 8: Using the try_catch wrapper
    std::println("\n=== Example 8: Using the try_catch() wrapper ===");
    example_with_try_catch();

    // Example 9: Function that returns error_code directly
    std::println("\n=== Example 9: Function that returns error_code directly ===");
    if (auto ec = allocate_large_buffer(2'000'000'000)) {
        std::println("Error: {} (code: {}).", ec.message(), ec.value());

        // Get the corresponding error condition
        auto condition = ec.default_error_condition();
        std::println("Error condition: {} ({}).", condition.message(), condition.value());
    }

    // Example 10: Using first_of to try multiple locations
    std::println("\n=== Example 10: Using first_of to try multiple locations ===");
    if (auto config_result = read_config_file()) {
        std::println("Config loaded successfully.");
    } else {
        std::println("Failed to load config: {}", config_result.error().message());
    }

    return 0;
}
