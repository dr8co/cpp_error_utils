#pragma once

#include <string>
#include <system_error>


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
    runtime_error,   ///< Errors that occur during program execution.
    resource_error,  ///< Errors related to resource allocation and management.
    access_error,    ///< Errors related to invalid access of data structures.
    other_error,     ///< Other errors that do not fit into the above categories.
};

// clang-format on
// @formatter:on

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

// Singleton for the condition category
inline const std::error_category &extra_error_condition_category() {
    static ExtraErrorConditionCategory instance;
    return instance;
}

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

    // Add mapping from error codes to error conditions
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
                return {static_cast<int>(ExtraErrorCondition::resource_error), extra_error_condition_category()};

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

inline const std::error_category &extra_error_category() {
    static ExtraErrorCategory instance;
    return instance;
}

template <>
struct std::is_error_code_enum<ExtraError> : std::true_type {};

template <>
struct std::is_error_condition_enum<ExtraErrorCondition> : std::true_type {};

inline std::error_code make_error_code(ExtraError e) {
    return {static_cast<int>(e), extra_error_category()};
}

inline std::error_condition make_error_condition(ExtraErrorCondition e) {
    return {static_cast<int>(e), extra_error_condition_category()};
}
