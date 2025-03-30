#include <error_utils.hpp>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

// Example: Wrapper for C file API
StringResult read_file_c_api(const std::string &filename) {
    // Open a file using C API
    errno = 0;
    const int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        return make_error_from_errno<std::string>(std::format("Failed to open '{}'", filename));
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
            const auto err = last_error();
            close(fd);
            return make_error<std::string>(err, std::format("Error reading '{}'", filename));
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
        return make_error<std::string>(std::errc::no_such_file_or_directory,
                                       std::format("Cannot open '{}'", filename));
    }

    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + '\n';
    }

    if (file.bad()) {
        return make_error<std::string>(std::errc::io_error,
                                       std::format("Error reading '{}'", filename));
    }

    return content;
}

// Example: Custom error checking logic
IntResult parse_positive_number(const std::string &str) {
    try {
        int value = std::stoi(str);

        if (value < 0) {
            return make_error<int>(std::errc::invalid_argument,
                                   "Number must be positive");
        }

        return value;
    } catch (const std::invalid_argument &) {
        return make_error<int>(std::errc::invalid_argument,
                               "Invalid number format");
    } catch (const std::out_of_range &) {
        return make_error<int>(std::errc::result_out_of_range,
                               "Number out of range");
    }
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

int main() {
    // Example 1: Reading with C API
    if (auto c_result = read_file_c_api("/etc/passwd")) {
        std::cout << "File size: " << c_result.value().size() << " bytes\n";
    } else {
        std::cout << "Error: " << c_result.error().message() << std::endl;
    }

    // Example 2: Reading with C++ API
    if (auto cpp_result = read_file_cpp_api("nonexistent.txt"); !cpp_result) {
        std::cout << "Error: " << cpp_result.error().message() << std::endl;

        // Check specific error
        if (cpp_result.error().is(std::errc::no_such_file_or_directory)) {
            std::cout << "The file doesn't exist\n";
        }
    }

    // Example 3: Using with custom logic
    if (auto num_result = parse_positive_number("-42"); !num_result) {
        std::cout << "Parse error: " << num_result.error().message() << std::endl;
    }

    // Example 4: Error chaining
    if (auto size_result = get_file_size("sample.txt"); !size_result) {
        std::cout << "Failed to get size: " << size_result.error().message() << std::endl;
    }

    return 0;
}
