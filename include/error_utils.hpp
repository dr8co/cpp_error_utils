#pragma once

#include <cerrno>
#include <chrono>
#include <expected>
#include <format>
#include <functional>
#include <future>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>
#include "extra_error.hpp"

/// Contains utilities for error handling and classification.
namespace error_utils {
    namespace detail {
        /// Helper struct to detect if a type is a std::expected specialization
        template <typename>
        struct is_expected : std::false_type {};

        template <typename T, typename E>
        struct is_expected<std::expected<T, E>> : std::true_type {};
    } // namespace detail

    /// A wrapper class for system error codes with additional context.
    class Error {
        std::string context_{};
        std::error_code error_code_{};

    public:
        /// Create an error with the specified error code and optional context.
        /// \param code The system error code
        /// \param context Additional context information about the error
        explicit Error(const std::error_code &code, const std::string_view context = {})
            : context_{context}, error_code_{code} {}

        template <typename T>
            requires std::is_error_condition_enum_v<T> || std::is_error_code_enum_v<T>
        explicit Error(const T code, const std::string_view context = {})
            : context_{context}, error_code_{make_error_code(code)} {}

        Error(const Error &other) = default;

        Error(Error &&other) noexcept
            : context_{std::move(other.context_)},
              error_code_{other.error_code_} {}

        Error &operator=(const Error &other) {
            if (this == &other)
                return *this;
            context_ = other.context_;
            error_code_ = other.error_code_;
            return *this;
        }

        Error &operator=(Error &&other) noexcept {
            if (this == &other)
                return *this;
            context_ = std::move(other.context_);
            error_code_ = other.error_code_;
            return *this;
        }

        /// Implicit conversion to bool, indicating whether an error exists.
        [[nodiscard]] explicit operator bool() const noexcept {
            return error_code_.operator bool();
        }

        void set_context(const std::string_view context) { context_ = context; }
        void set_error_code(const std::error_code &error_code) { error_code_ = error_code; }

        /// Get the underlying error code.
        /// \return The error code
        [[nodiscard]] std::error_code code() const noexcept { return error_code_; }

        [[nodiscard]] std::string context() const { return context_; }

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
        template <typename T>
            requires std::is_error_condition_enum_v<T> || std::is_error_code_enum_v<T>
        [[nodiscard]] bool is(const T code) const noexcept {
            return error_code_ == make_error_code(code);
        }

        [[nodiscard]] bool is(const std::error_code &code) const noexcept {
            return error_code_ == code;
        }

        /// Check if the error belongs to any of the specified error codes.
        /// \param codes List of error codes to check against
        /// \return True if the error matches any of the specified codes
        template <typename... Codes>
        [[nodiscard]] bool is_any_of(Codes... codes) const noexcept {
            return (is(codes) || ...);
        }

        friend void swap(Error &lhs, Error &rhs) noexcept {
            std::swap(lhs.context_, rhs.context_);
            std::swap(lhs.error_code_, rhs.error_code_);
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
    template <typename T, typename E>
        requires std::is_error_condition_enum_v<E> || std::is_error_code_enum_v<E>
    [[nodiscard]] Result<T> make_error(const E code, const std::string_view context = {}) {
        return std::unexpected(Error{code, context});
    }

    template <typename T>
    [[nodiscard]] Result<T> make_error(const std::error_code &code, const std::string_view context = {}) {
        return std::unexpected(Error{code, context});
    }

    template <typename T>
    [[nodiscard]] Result<T> make_error(const std::regex_constants::error_type &code,
                                       const std::string_view context = {}) {
        std::errc ec{};
        const char *detailed_msg{};
        switch (code) {
            case std::regex_constants::error_collate:
                detailed_msg = "Regex error: invalid collating element name";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_ctype:
                detailed_msg = "Regex error: invalid character class name";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_escape:
                detailed_msg = "Regex error: invalid escaped character or a trailing escape";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_backref:
                detailed_msg = "Regex error: invalid back reference";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_brack:
                detailed_msg = "Regex error: mismatched square brackets ('[' and ']')";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_paren:
                detailed_msg = "Regex error: mismatched parentheses ('(' and ')')";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_brace:
                detailed_msg = "Regex error: mismatched curly braces ('{' and '}')";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_badbrace:
                detailed_msg = "Regex error: invalid range in a {} expression";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_range:
                detailed_msg = "Regex error: invalid character range";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_space:
                detailed_msg = "Regex error: insufficient memory to convert the expression into a finite state machine";
                ec = std::errc::not_enough_memory;
                break;
            case std::regex_constants::error_badrepeat:
                detailed_msg = "Regex error: '*', '?', '+' or '{' was not preceded by a valid regular expression";
                ec = std::errc::invalid_argument;
                break;
            case std::regex_constants::error_complexity:
                detailed_msg = "Regex error: the complexity of an attempted match exceeded a predefined level";
                ec = std::errc::result_out_of_range;
                break;
            case std::regex_constants::error_stack:
                detailed_msg = "Regex error: insufficient memory to perform a match";
                ec = std::errc::not_enough_memory;
                break;
            default:
                detailed_msg = "Regex error: unknown error";
                ec = std::errc::invalid_argument;
                break;
        }

        const std::string full_context = context.empty() ? detailed_msg : std::string(context) + ": " + detailed_msg;
        return std::unexpected(Error{ec, full_context});
    }


    /// Create an error result from the current errno value.
    /// \param context Optional context information
    /// \return An unexpected result with the current errno
    template <typename T>
    [[nodiscard]] Result<T> make_error_from_errno(const std::string_view context = {}) {
        return make_error<T>(last_error(), context);
    }

    /// Apply a transformation function to a successful Result.
    /// \param result Original result
    /// \param transform_func Function to transform value
    /// \return New result with the transformed value or forwarded error
    template <typename T, typename Func>
    [[nodiscard]] auto transform(Result<T> result, Func &&transform_func) -> Result<std::invoke_result_t<Func, T>> {
        using R = std::invoke_result_t<Func, T>;

        if (result) {
            if constexpr (std::is_void_v<T>) {
                return Result<R>(std::forward<Func>(transform_func)());
            } else {
                return Result<R>(std::forward<Func>(transform_func)(result.value()));
            }
        }
        return std::unexpected(result.error());
    }

    /// Run a function on a successful Result, which may itself return a Result.
    /// This allows for easy chaining of fallible operations.
    /// \param result Original result
    /// \param func Function that returns a new Result
    /// \return Result from func or forwarded error
    template <typename T, typename Func>
    [[nodiscard]] auto and_then(Result<T> result, Func &&func) -> std::invoke_result_t<Func, T> {
        using R = std::invoke_result_t<Func, T>;
        static_assert(detail::is_expected<R>::value, "func must return a Result<U, Error>");

        if (result) {
            if constexpr (std::is_void_v<T>) {
                return std::forward<Func>(func)();
            } else {
                return std::forward<Func>(func)(result.value());
            }
        }
        return std::unexpected(result.error());
    }


    /// Recover from an error by providing a fallback value.
    /// \param result Original result
    /// \param fallback Value to use if the result contains an error
    /// \return Either the original value or the fallback
    template <typename T>
    [[nodiscard]] T or_else(Result<T> result, T fallback) {
        if (result) {
            return result.value();
        }
        return fallback;
    }


    /// Recover from an error by calling a function to provide a fallback value.
    /// \param result Original result
    /// \param recover_func Function to call with the error to generate fallback
    /// \return Either the original value or the recovered value
    template <typename T, typename Func>
    [[nodiscard]] T or_else_with(Result<T> result, Func &&recover_func) {
        if (result) {
            return result.value();
        }
        return std::forward<Func>(recover_func)(result.error());
    }


    /// Execute a function and catch common exceptions, converting them to errors.
    /// \param func Function to execute
    /// \param context Error context
    /// \return Result of the function or an error from caught exceptions
    template <typename Func>
    [[nodiscard]] auto try_catch(Func &&func, std::string_view context = {}) -> Result<std::invoke_result_t<Func>> {
        using R = std::invoke_result_t<Func>;

        static_assert(detail::is_expected<R>::value, "func must return a Result<T, Error>");

        auto format_message = [&context](auto &&default_msg) -> std::string {
            return context.empty() ? default_msg : std::format("{}: {}", context, default_msg);
        };

        try {
            return std::forward<Func>(func)();

            // Logic errors
        } catch (const std::invalid_argument &e) {
            return make_error<R>(ExtraError::invalid_argument, format_message(e.what()));
        } catch (const std::domain_error &e) {
            return make_error<R>(std::errc::argument_out_of_domain, format_message(e.what()));
        } catch (const std::length_error &e) {
            return make_error<R>(ExtraError::length_error, format_message(e.what()));
        } catch (const std::out_of_range &e) {
            return make_error<R>(std::errc::result_out_of_range, format_message(e.what()));
        } catch (const std::future_error &e) {
            return make_error<R>(e.code(), format_message(e.what()));

            // Runtime errors
        } catch (const std::range_error &e) {
            return make_error<R>(std::errc::result_out_of_range, format_message(e.what()));
        } catch (const std::overflow_error &e) {
            return make_error<R>(std::errc::value_too_large, format_message(e.what()));
        } catch (const std::underflow_error &e) {
            return make_error<R>(ExtraError::value_too_small, format_message(e.what()));
        } catch (const std::regex_error &e) {
            return make_error<R>(e.code(), format_message(e.what()));
        } catch (const std::system_error &e) {
            return make_error<R>(e.code(), format_message(e.what()));
        } catch (const std::chrono::nonexistent_local_time &e) {
            return make_error<R>(ExtraError::nonexistent_local_time, format_message(e.what()));
        } catch (const std::chrono::ambiguous_local_time &e) {
            return make_error<R>(ExtraError::ambiguous_local_time, format_message(e.what()));
        } catch (const std::format_error &e) {
            return make_error<R>(ExtraError::format_error, format_message(e.what()));

            // Resource and type errors
        } catch (const std::bad_alloc &e) {
            return make_error<R>(ExtraError::bad_alloc, format_message(e.what()));
        } catch (const std::bad_typeid &e) {
            return make_error<R>(ExtraError::bad_typeid, format_message(e.what()));
        } catch (const std::bad_cast &e) {
            return make_error<R>(ExtraError::bad_cast, format_message(e.what()));

            // Container and value access errors
        } catch (const std::bad_optional_access &e) {
            return make_error<R>(ExtraError::bad_optional_access, format_message(e.what()));
        } catch (const std::bad_expected_access<void> &e) {
            return make_error<R>(ExtraError::bad_expected_access, format_message(e.what()));
        } catch (const std::bad_variant_access &e) {
            return make_error<R>(ExtraError::bad_variant_access, format_message(e.what()));
        } catch (const std::bad_weak_ptr &e) {
            return make_error<R>(ExtraError::bad_weak_ptr, format_message(e.what()));
        } catch (const std::bad_function_call &e) {
            return make_error<R>(ExtraError::bad_function_call, format_message(e.what()));
        } catch (const std::bad_exception &e) {
            return make_error<R>(ExtraError::bad_exception, format_message(e.what()));

            // Catch-all for any other exceptions
        } catch (const std::exception &e) {
            return make_error<R>(ExtraError::exception, format_message(e.what()));
        } catch (...) {
            return make_error<R>(ExtraError::unknown_exception, context);
        }
    }


    /// Maps different error codes to a common error.
    /// Useful for unifying error handling across different APIs.
    /// \param result Original result
    /// \param map_func Function that maps errors to new errors
    /// \return Result with the same value or mapped error
    template <typename T>
    [[nodiscard]] Result<T> map_error(Result<T> result, const std::function<Error(const Error &)> &map_func) {
        if (result) {
            return result;
        }
        return std::unexpected(map_func(result.error()));
    }


    /// Return first success result from multiple alternatives
    /// \param results Multiple results of the same type
    /// \return First successful result or combined error
    template <typename T>
    [[nodiscard]] Result<T> first_of(std::initializer_list<Result<T>> results) {
        std::string combined_errors;

        for (const auto &result : results) {
            if (result) {
                return result;
            }
            if (!combined_errors.empty()) {
                combined_errors += "; ";
            }
            combined_errors += result.error().message();
        }

        return make_error<T>(std::errc::io_error, combined_errors);
    }

    /// Execute a function that may set errno, capturing the result and any error.
    ///
    /// \param func Function that may set errno
    /// \param error_context Context to use if an error occurs
    /// \return Result of the function or an error if errno was set
    template <typename Func, typename R = std::invoke_result_t<Func>>
    auto with_errno(Func &&func, const std::string_view error_context = {}) -> Result<R> {
        errno = 0;

        if constexpr (std::is_void_v<R>) {
            std::forward<Func>(func)();
            if (errno != 0) {
                return std::unexpected(Error(last_error(), error_context));
            }
            return {};
        } else {
            R result = std::forward<Func>(func)();
            if (errno != 0) {
                return std::unexpected(Error(last_error(), error_context));
            }
            return result;
        }
    }


    /// Check if an operation that may return no value succeeded.
    /// Useful for C functions that return -1 or nullptr on error.
    ///
    /// \tparam T Value type
    /// \param value Value to check
    /// \param error_detector Function that detects if the value indicates an error
    /// \param error_code Error code to use if value indicates error
    /// \param context Error context
    /// \return Result with either the value or an error
    template <typename T>
    [[nodiscard]] Result<T> check_value(T value, std::function<bool(const T &)> error_detector,
                                        const std::errc error_code,
                                        const std::string_view context = {}) {
        if (error_detector(value)) {
            return make_error<T>(error_code, context);
        }
        return value;
    }


    /// Specialized function for checking integer return codes from C APIs.
    /// Many C functions return -1 on error and set errno.
    ///
    /// \param return_code Integer return code from C function
    /// \param context Error context
    /// \return Result with either void (success) or an error
    [[nodiscard]] inline VoidResult check_return_code(const int return_code, const std::string_view context = {}) {
        if (return_code == -1) {
            return make_error_from_errno<void>(context);
        }
        return {};
    }
}

// Common result type aliases
using error_utils::Result;
using error_utils::VoidResult;
using error_utils::StringResult;
using error_utils::IntResult;
using error_utils::BoolResult;
