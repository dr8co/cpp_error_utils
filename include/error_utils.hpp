#pragma once

#include <cerrno>
#include <expected>
#include <format>
#include <ostream>
#include <string>
#include <string_view>
#include <system_error>

template <typename T>
concept ErrorCode = requires(T t) {
    { std::make_error_code(t) } -> std::same_as<std::error_code>;
};

/// A wrapper class for system error codes with additional context.
class Error {
    std::string context_{};
    std::error_code error_code_{};

public:
    /// Create an error with the specified error code and optional context.
    /// \param code The system error code
    /// \param context Additional context information about the error
    explicit Error(const std::error_code& code, const std::string_view context = {})
        : context_{context}, error_code_{code} {}

    explicit Error(const ErrorCode auto code, const std::string_view context = {})
        : context_{context}, error_code_{std::make_error_code(code)} {}

    Error(const Error& other) = default;

    Error(Error&& other) noexcept
        : context_{std::move(other.context_)},
          error_code_{other.error_code_} {}

    Error& operator=(const Error& other) {
        if (this == &other)
            return *this;
        context_ = other.context_;
        error_code_ = other.error_code_;
        return *this;
    }

    Error& operator=(Error&& other) noexcept {
        if (this == &other)
            return *this;
        context_ = std::move(other.context_);
        error_code_ = other.error_code_;
        return *this;
    }

    void set_context(const std::string_view context) { context_ = context; }
    void set_error_code(const std::error_code& error_code) { error_code_ = error_code; }

    /// Get the underlying error code.
    /// \return The error code
    [[nodiscard]] std::error_code code() const noexcept { return error_code_; }

    [[nodiscard]] std::string context() const { return context_; }

    friend void swap(Error& lhs, Error& rhs) noexcept {
        std::swap(lhs.context_, rhs.context_);
        std::swap(lhs.error_code_, rhs.error_code_);
    }

    /// Get the error message including context if available.
    /// \return Formatted error message
    [[nodiscard]] std::string message() const {
        if (context_.empty()) {
            return error_code_.message();
        }
        return std::format("{}: {}", context_, error_code_.message());
    }

    /// Check if the error is of a specific type.
    /// \param code The error code to check against
    /// \return True if the error matches the specified code
    [[nodiscard]] bool is(const std::errc code) const noexcept {
        return error_code_ == std::make_error_code(code);
    }

    [[nodiscard]] bool is(const std::error_code& code) const noexcept {
        return error_code_ == code;
    }

    /// Check if the error belongs to any of the specified error codes.
    /// \param codes List of error codes to check against
    /// \return True if the error matches any of the specified codes
    template <typename... Codes>
    [[nodiscard]] bool is_any_of(Codes... codes) const noexcept {
        return (is(codes) || ...);
    }
};


/// Retrieve the last system error code and reset errno.
/// \return The last system error code as std::errc
[[nodiscard]] inline std::error_code last_error() noexcept {
    int err = errno;
    errno = 0; // Reset errno to avoid side effects
    return std::make_error_code(static_cast<std::errc>(err));
}


// Define common result types using std::expected
template <typename T>
using Result = std::expected<T, Error>;

// Common result type aliases
using VoidResult = Result<void>;
using StringResult = Result<std::string>;
using IntResult = Result<int>;
using BoolResult = Result<bool>;


/// Create an error result of the specified type.
/// \param code The error code
/// \param context Optional context information
/// \return An unexpected result with the error
template <typename T>
[[nodiscard]] Result<T> make_error(const ErrorCode auto code, const std::string_view context = {}) {
    return std::unexpected(Error{code, context});
}

template <typename T>
[[nodiscard]] Result<T> make_error(const std::error_code& code, const std::string_view context = {}) {
    return std::unexpected(Error{code, context});
}


/// Create an error result from the current errno value.
/// \param context Optional context information
/// \return An unexpected result with the current errno
template <typename T>
[[nodiscard]] Result<T> make_error_from_errno(const std::string_view context = {}) {
    return make_error<T>(last_error(), context);
}
