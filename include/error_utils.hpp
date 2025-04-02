#pragma once

#include <cerrno>
#include <chrono>
#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <future>
#include <ostream>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>


// ///////////////////////// Error Codes, Conditions, and Categories ///////////////////////

/// Contains utilities for error handling and classification.
namespace error_utils {
// clang-format off
// @formatter:off

/// Represents specific error codes for exception handling and error classification.
///
/// This scoped enumeration defines detailed error codes that map to specific
/// C++ standard library exceptions, providing a unified way to handle various error types.
///
/// It is designed to work with the standard error handling facilities and integrates
/// with \p std::error_code and \p std::error_condition
enum class ExtraError {
    // Logic errors (std::logic_error exceptions)
    invalid_argument = 1,    ///< std::invalid_argument exception
    length_error,            ///< std::length_error exception

    // Runtime errors (std::runtime_error exceptions)
    value_too_small,         ///< std::underflow_error exception
    nonexistent_local_time,  ///< std::chrono::nonexistent_local_time exception
    ambiguous_local_time,    ///< std::chrono::ambiguous_local_time exception
    format_error,            ///< std::format_error exception

    // Resource and type exceptions
    bad_alloc,               ///< std::bad_alloc exception
    bad_typeid,              ///< std::bad_typeid exception
    bad_cast,                ///< std::bad_cast exception

    // Container and value access exceptions
    bad_optional_access,     ///< std::bad_optional_access exception
    bad_expected_access,     ///< std::bad_expected_access exception
    bad_variant_access,      ///< std::bad_variant_access exception
    bad_weak_ptr,            ///< std::bad_weak_ptr exception
    bad_function_call,       ///< std::bad_function_call exception

    // Other exceptions
    bad_exception,           ///< std::bad_exception exception
    exception,               ///< all std::exception exceptions
    unknown_exception,       ///< catch-all for any other exceptions
};


/// Represents categories of error conditions for error handling and classification.
///
/// This enumeration defines broad categories of errors that can occur in the system,
/// which are used to group specific error codes into more general error conditions.
enum class ExtraErrorCondition {
    logic_error = 1, ///< Errors related to program logic and invalid operations.
    runtime_error,   ///< Errors occurring during program execution.
    resource_error,  ///< Errors related to resource allocation and management.
    access_error,    ///< Errors related to invalid access of data structures.
    other_error,     ///< Other errors that do not fit into the above categories.
};

    // clang-format on
    // @formatter:on

    /// namespace for internal use: do not use directly.
    namespace detail {
        // Define the error condition category
        class ExtraErrorConditionCategory final : public std::error_category {
        public:
            [[nodiscard]] const char *name() const noexcept override {
                return "ExtraErrorCondition";
            }

            [[nodiscard]] std::string message(int ev) const override {
                switch (static_cast<ExtraErrorCondition>(ev)) {
                    case ExtraErrorCondition::logic_error:
                        return "Logic error";
                    case ExtraErrorCondition::runtime_error:
                        return "Runtime error";
                    case ExtraErrorCondition::resource_error:
                        return "Resource error";
                    case ExtraErrorCondition::access_error:
                        return "Access error";
                    case ExtraErrorCondition::other_error:
                        return "Other error";
                    default:
                        return "Unrecognized error condition";
                }
            }
        };

        /// Returns a reference to the \p ExtraErrorCondition error category.
        /// \return A singleton instance of \p ExtraErrorConditionCategory
        inline const std::error_category &extra_error_condition_category() {
            static ExtraErrorConditionCategory instance;
            return instance;
        }

        /// Error category for \p ExtraError
        /// This class provides a mapping from \p ExtraError enum values to error messages.
        /// It also provides a default error condition mapping for each error code.
        class ExtraErrorCategory final : public std::error_category {
        public:
            [[nodiscard]] const char *name() const noexcept override {
                return "ExtraError";
            }

            [[nodiscard]] std::string message(int ev) const override {
                switch (static_cast<ExtraError>(ev)) {
                    case ExtraError::invalid_argument:
                        return "Invalid argument exception";
                    case ExtraError::length_error:
                        return "Length error exception";

                    //
                    case ExtraError::value_too_small:
                        return "Value too small (underflow exception)";
                    case ExtraError::nonexistent_local_time:
                        return "Nonexistent local time exception";
                    case ExtraError::ambiguous_local_time:
                        return "Ambiguous local time exception";
                    case ExtraError::format_error:
                        return "Format error exception";

                    //
                    case ExtraError::bad_alloc:
                        return "Bad allocation exception";
                    case ExtraError::bad_typeid:
                        return "Bad typeid exception";
                    case ExtraError::bad_cast:
                        return "Bad cast exception";

                    //
                    case ExtraError::bad_optional_access:
                        return "Bad optional access exception";
                    case ExtraError::bad_expected_access:
                        return "Bad expected access exception";
                    case ExtraError::bad_variant_access:
                        return "Bad variant access exception";
                    case ExtraError::bad_weak_ptr:
                        return "Bad weak pointer exception";
                    case ExtraError::bad_function_call:
                        return "Bad function call exception";

                    //
                    case ExtraError::bad_exception:
                        return "Bad exception";
                    case ExtraError::exception:
                        return "Exception caught";
                    case ExtraError::unknown_exception:
                        return "Unknown exception caught";
                    default:
                        return "Unrecognized ExtraError";
                }
            }

            /// Provides the default error condition for the given error code.
            /// \param ev The error code to map to an error condition.
            /// \return The corresponding error condition.
            [[nodiscard]] std::error_condition default_error_condition(int ev) const noexcept override {
                switch (static_cast<ExtraError>(ev)) {
                    case ExtraError::invalid_argument:
                    case ExtraError::length_error:
                        return {static_cast<int>(ExtraErrorCondition::logic_error), extra_error_condition_category()};

                    case ExtraError::value_too_small:
                    case ExtraError::nonexistent_local_time:
                    case ExtraError::ambiguous_local_time:
                    case ExtraError::format_error:
                        return {static_cast<int>(ExtraErrorCondition::runtime_error), extra_error_condition_category()};

                    case ExtraError::bad_alloc:
                    case ExtraError::bad_typeid:
                    case ExtraError::bad_cast:
                        return {
                            static_cast<int>(ExtraErrorCondition::resource_error), extra_error_condition_category()
                        };

                    case ExtraError::bad_optional_access:
                    case ExtraError::bad_expected_access:
                    case ExtraError::bad_variant_access:
                    case ExtraError::bad_weak_ptr:
                    case ExtraError::bad_function_call:
                        return {static_cast<int>(ExtraErrorCondition::access_error), extra_error_condition_category()};

                    case ExtraError::bad_exception:
                    case ExtraError::exception:
                    case ExtraError::unknown_exception:
                    default:
                        return {static_cast<int>(ExtraErrorCondition::other_error), extra_error_condition_category()};
                }
            }
        };

        /// This function provides a singleton instance of the \p ExtraErrorCategory class,
        /// @return A singleton reference to the \p ExtraError error category.
        inline const std::error_category &extra_error_category() {
            static ExtraErrorCategory instance;
            return instance;
        }
    } // namespace detail


    /// Create an error code from an \p ExtraError enum value.
    /// \param e The \p ExtraError enum value
    constexpr std::error_code make_error_code(ExtraError e) {
        return {static_cast<int>(e), detail::extra_error_category()};
    }

    /// Create an error condition from an \p ExtraErrorCondition enum value.
    /// \param e The \p ExtraErrorCondition enum value
    constexpr std::error_condition make_error_condition(ExtraErrorCondition e) {
        return {static_cast<int>(e), detail::extra_error_condition_category()};
    }
} // namespace error_utils


// STL customization points
namespace std {
    template <>
    struct is_error_code_enum<error_utils::ExtraError> : true_type {};

    template <>
    struct is_error_condition_enum<error_utils::ExtraErrorCondition> : true_type {};
} // namespace std

using error_utils::ExtraError;
using error_utils::ExtraErrorCondition;


// ///////////////////////// Error Handling Utilities /////////////////////////


namespace error_utils {
    namespace detail {
        /// Type trait to check if a type is an expected type.
        template <typename>
        struct is_expected : std::false_type {};

        template <typename T, typename E>
        struct is_expected<std::expected<T, E>> : std::true_type {};

        /// Helper variable template for \p is_expected.
        template <typename T>
        inline constexpr bool is_expected_v = is_expected<T>::value;

        /// A concept to check if a type is convertible to \p std::error_code.
        template <typename T>
        concept convertible_to_error_code = (std::is_error_condition_enum_v<T> || std::is_error_code_enum_v<T>) &&
            requires { { make_error_code(std::declval<T>()) } -> std::same_as<std::error_code>; };
    } // namespace detail

    /// A wrapper class for system error codes with additional context.
    class Error {
        std::string context_{};
        std::error_code error_code_{};

    public:
        Error() = default;

        /// Create an error with the specified error code and optional context.
        /// \param code The system error code
        /// \param context Additional context information about the error
        explicit Error(const std::error_code &code, const std::string_view context = {})
            : context_{context}, error_code_{code} {}

        explicit Error(const detail::convertible_to_error_code auto code, const std::string_view context = {})
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

        ~Error() = default;

        constexpr friend bool operator==(const Error &lhs, const Error &rhs) noexcept {
            return lhs.error_code_ == rhs.error_code_;
        }

        constexpr friend auto operator<=>(const Error &lhs, const Error &rhs) noexcept {
            return lhs.error_code_ <=> rhs.error_code_;
        }

        constexpr friend bool operator==(const Error &lhs, const std::error_code &rhs) noexcept {
            return lhs.error_code_ == rhs;
        }

        constexpr friend auto operator<=>(const Error &lhs, const std::error_code &rhs) noexcept {
            return lhs.error_code_ <=> rhs;
        }

        constexpr friend bool operator==(const Error &lhs, const std::error_condition &rhs) noexcept {
            return lhs.error_code_ == rhs;
        }

        constexpr friend std::ostream &operator<<(std::ostream &os, const Error &obj) {
            return os
                << obj.message()
                << "\n(error_code: " << obj.error_code_.value() << " ("
                << obj.error_code_.category().name() << " category))";
        }

        /// Implicit conversion to bool, indicating whether an error exists.
        [[nodiscard]] explicit operator bool() const noexcept {
            return error_code_.operator bool();
        }

        // void set_context(const std::string_view context) { context_ = context; }
        // void set_error_code(const std::error_code &error_code) { error_code_ = error_code; }

        /// Get the underlying error code.
        /// \return The error code
        [[nodiscard]] const std::error_code &error_code() const noexcept { return error_code_; }

        [[nodiscard]] const std::string &context() const noexcept { return context_; }

        [[nodiscard]] int value() const noexcept { return error_code_.value(); }

        [[nodiscard]] const std::error_category &category() const noexcept {
            return error_code_.category();
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
        template <typename T>
            requires detail::convertible_to_error_code<T>
        [[nodiscard]] bool is(T &&code) const noexcept {
            using std::make_error_code;
            return error_code_ == make_error_code(std::forward<T>(code));
        }

        [[nodiscard]] bool is(const std::error_code &code) const noexcept {
            return error_code_ == code;
        }

        /// Check if the error belongs to any of the specified error codes.
        /// \param code The first error code to check against
        /// \param others Other error codes to check against
        /// \tparam Code The type of the first error code
        /// \tparam Others The types of the other error codes
        /// \return True if the error matches any of the specified codes
        template <typename Code, typename... Others>
            requires (detail::convertible_to_error_code<Code> || std::is_same_v<Code, std::error_code>) &&
            ((detail::convertible_to_error_code<Others> || std::is_same_v<Code, std::error_code>) && ...)
        [[nodiscard]] bool is_any_of(Code &&code, Others &&... others) const noexcept {
            return is(std::forward<Code>(code)) || (is(std::forward<Others>(others)) || ...);
            // return (is(std::forward<Code>(code)) || ... || is(std::forward<Others>(others)));
        }

        friend void swap(Error &lhs, Error &rhs) noexcept {
            using std::swap;
            swap(lhs.context_, rhs.context_);
            swap(lhs.error_code_, rhs.error_code_);
        }
    };


    // Define common result types using std::expected
    template <typename T = void>
    using Result = std::expected<T, Error>;

    // Common result type aliases
    using VoidResult = Result<>;
    using StringResult = Result<std::string>;
    using IntResult = Result<int>;
    using BoolResult = Result<bool>;


    /// Create an error result of the specified type.
    /// \param code The error code
    /// \param context Optional context information
    /// \return An unexpected result with the error
    template <typename T, typename E>
        requires detail::convertible_to_error_code<E>
    [[nodiscard]] Result<T> make_error(E &&code, const std::string_view context = {}) {
        return std::unexpected(Error{std::forward<E>(code), context});
    }

    template <typename T>
    [[nodiscard]] Result<T> make_error(const std::error_code &code, const std::string_view context = {}) {
        return std::unexpected(Error{code, context});
    }

    template <typename T>
    [[nodiscard]] Result<T> make_error(const std::regex_constants::error_type code,
                                       const std::string_view context = {}) {
        auto create_unexpected = [&context](const std::errc &err_code, const std::string_view msg) {
            return std::unexpected(Error{err_code, context.empty() ? msg : std::format("{}: {}", context, msg)});
        };

        // Map regex error codes to std::error_code
        switch (code) {
            case std::regex_constants::error_collate:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: invalid collating element name");

            case std::regex_constants::error_ctype:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: invalid character class name");

            case std::regex_constants::error_escape:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: invalid escaped character or a trailing escape");

            case std::regex_constants::error_backref:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: invalid back reference");

            case std::regex_constants::error_brack:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: mismatched square brackets ('[' and ']')");

            case std::regex_constants::error_paren:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: mismatched parentheses ('(' and ')')");

            case std::regex_constants::error_brace:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: mismatched curly braces ('{' and '}')");

            case std::regex_constants::error_badbrace:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: invalid range in a {} expression");

            case std::regex_constants::error_range:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: invalid character range");

            case std::regex_constants::error_space:
                return create_unexpected(std::errc::not_enough_memory,
                                         "Regex error: insufficient memory to convert the expression"
                                         " into a finite state machine");

            case std::regex_constants::error_badrepeat:
                return create_unexpected(std::errc::invalid_argument,
                                         "Regex error: '*', '?', '+' or '{' was not preceded"
                                         " by a valid regular expression");

            case std::regex_constants::error_complexity:
                return create_unexpected(std::errc::result_out_of_range,
                                         "Regex error: the complexity of an attempted match"
                                         " exceeded a predefined level");

            case std::regex_constants::error_stack:
                return create_unexpected(std::errc::not_enough_memory,
                                         "Regex error: insufficient memory to perform a match");

            default:
                return create_unexpected(std::errc::invalid_argument, "Regex error: unknown error");
        }
    }

    /// Retrieve the last system error code and reset \p errno.
    /// \return The last system error code as \p std::error_code
    [[nodiscard]] inline std::error_code last_error() noexcept {
        int err = errno;
        errno = 0; // Reset errno to avoid side effects
        return std::make_error_code(static_cast<std::errc>(err));
    }

    /// Create an error result from the current errno value.
    /// \param context Optional context information
    /// \return An unexpected result with the current \p errno
    template <typename T>
    [[nodiscard]] Result<T> make_error_from_errno(const std::string_view context = {}) {
        return make_error<T>(last_error(), context);
    }

    /// Execute a function that may set errno, capturing the result and any error.
    ///
    /// \note The errno value is reset before and after the function call.
    ///
    /// \param func Function that may set errno
    /// \param error_context Context to use if an error occurs
    /// \return Result of the function or an error if errno was set
    template <typename Func, typename R = std::invoke_result_t<Func>>
    auto with_errno(Func &&func, const std::string_view error_context = {}) -> Result<R> {
        // Reset errno before calling the function to avoid side effects
        errno = 0;

        if constexpr (std::is_void_v<R>) {
            std::forward<Func>(func)();
            if (errno != 0) {
                return make_error_from_errno<void>(error_context);
            }
            return {};
        } else {
            R result = std::forward<Func>(func)();
            if (errno != 0) {
                return make_error_from_errno<R>(error_context);
            }
            return result;
        }
    }

    /// Execute a function that may set \p errno, capturing the result and any error.
    ///
    /// This function is intended for use with system calls that return an integer result,
    /// where a return value of -1 indicates an error.
    ///
    /// \param func Function that may set errno. Expected to return an integral type convertible to int.
    /// \tparam Func Type of the function to execute
    /// \param error_context Context to use if an error occurs
    /// \return Result of the function or an error if errno was set
    /// \note The \p errno value is reset before and after the function call.
    /// \note The function must be \p noexcept to ensure that it does not throw exceptions.
    /// \note Notice that the function must be invocable with no arguments.

    /// Use a lambda or \p std::bind to wrap the function.
    template <typename Func>
        requires std::is_nothrow_invocable_v<Func>
    IntResult invoke_with_syscall_api(Func &&func, const std::string_view error_context = {}) noexcept {
        using R = std::invoke_result_t<Func>;
        static_assert(std::is_integral_v<R> && std::convertible_to<R, int>,
                      "func must return an integral type convertible to int");

        // Reset errno before calling the function to avoid side effects
        errno = 0;

        R result = std::forward<Func>(func)();
        if (result == -1) {
            return make_error_from_errno<int>(error_context);
        }

        return result;
    }

    /// Execute a function and catch common exceptions, converting them to errors.
    /// \param func Function to execute
    /// \param context Error context
    /// \return Result of the function or an error from caught exceptions
    template <typename Func, typename R = std::invoke_result_t<Func>>
    [[nodiscard]] auto try_catch(Func &&func, std::string_view context = {}) -> Result<R> {
        auto create_error = [&context]<typename T>(T &&code, const std::string_view default_msg) -> Result<R> {
            return make_error<R>(std::forward<T>(code),
                                 context.empty() ? default_msg : std::format("{}: {}", context, default_msg));
        };

        try {
            return std::forward<Func>(func)();

            // Logic errors
        } catch (const std::invalid_argument &e) {
            return create_error(ExtraError::invalid_argument, e.what());
        } catch (const std::domain_error &e) {
            return create_error(std::errc::argument_out_of_domain, e.what());
        } catch (const std::length_error &e) {
            return create_error(ExtraError::length_error, e.what());
        } catch (const std::out_of_range &e) {
            return create_error(std::errc::result_out_of_range, e.what());
        } catch (const std::future_error &e) {
            return create_error(e.code(), e.what());

            // Runtime errors
        } catch (const std::range_error &e) {
            return create_error(std::errc::result_out_of_range, e.what());
        } catch (const std::overflow_error &e) {
            return create_error(std::errc::value_too_large, e.what());
        } catch (const std::underflow_error &e) {
            return create_error(ExtraError::value_too_small, e.what());
        } catch (const std::regex_error &e) {
            return create_error(e.code(), e.what());
        } catch (const std::system_error &e) {
            return create_error(e.code(), e.what());
        } catch (const std::chrono::nonexistent_local_time &e) {
            return create_error(ExtraError::nonexistent_local_time, e.what());
        } catch (const std::chrono::ambiguous_local_time &e) {
            return create_error(ExtraError::ambiguous_local_time, e.what());
        } catch (const std::format_error &e) {
            return create_error(ExtraError::format_error, e.what());

            // Resource and type errors
        } catch (const std::bad_alloc &e) {
            return create_error(ExtraError::bad_alloc, e.what());
        } catch (const std::bad_typeid &e) {
            return create_error(ExtraError::bad_typeid, e.what());
        } catch (const std::bad_cast &e) {
            return create_error(ExtraError::bad_cast, e.what());

            // Container and value access errors
        } catch (const std::bad_optional_access &e) {
            return create_error(ExtraError::bad_optional_access, e.what());
        } catch (const std::bad_expected_access<void> &e) {
            return create_error(ExtraError::bad_expected_access, e.what());
        } catch (const std::bad_variant_access &e) {
            return create_error(ExtraError::bad_variant_access, e.what());
        } catch (const std::bad_weak_ptr &e) {
            return create_error(ExtraError::bad_weak_ptr, e.what());
        } catch (const std::bad_function_call &e) {
            return create_error(ExtraError::bad_function_call, e.what());
        } catch (const std::bad_exception &e) {
            return create_error(ExtraError::bad_exception, e.what());

            // Catch-all for any other exceptions
        } catch (const std::exception &e) {
            return create_error(ExtraError::exception, e.what());
        } catch (...) {
            return create_error(ExtraError::unknown_exception, "Unknown exception");
        }
    }

    /// Return first success result from multiple alternatives
    /// \param results Multiple results of the same type
    /// \return First successful result or combined error
    template <typename T>
    [[nodiscard]] Result<T> first_of(std::initializer_list<Result<T>> results) {
        std::ostringstream combined_errors;

        for (const auto &result : results) {
            if (result) {
                return result;
            }
            if (combined_errors.tellp() > 0) {
                combined_errors << "; ";
            }
            combined_errors << result.error().message();
        }

        return make_error<T>(std::errc::io_error, combined_errors.str());
    }
} // namespace error_utils

namespace std {
    template <>
    struct formatter<error_utils::Error> {
        static constexpr auto parse(format_parse_context &ctx) {
            return ctx.begin();
        }

        static auto format(const error_utils::Error &error, format_context &ctx) {
            return format_to(ctx.out(), "{} \n(error_code: {}, category: {})",
                             error.message(), error.value(), error.category().name());
        }
    };

    template <>
    struct hash<error_utils::Error> {
        size_t operator()(const error_utils::Error &error) const noexcept {
            return hash<error_code>{}(error.error_code());
        }
    };
}


// ///////////////////////// Common Result Type Aliases /////////////////////////

using error_utils::Result;
using error_utils::VoidResult;
using error_utils::StringResult;
using error_utils::IntResult;
using error_utils::BoolResult;
