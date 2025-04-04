#include <error_utils.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace error_utils;

// ///////////////// Tests on utils::Error class /////////////////////////

// Test for Error class default construction
TEST(ErrorTest, DefaultConstruction) {
    const Error error;
    EXPECT_FALSE(error); // Default error should evaluate to false
    EXPECT_EQ(error.value(), 0);
    // EXPECT_EQ(error.message(), "Success"); // message is platform-dependent
}

TEST(ErrorTest, CopyConstruction) {
    const Error original(std::make_error_code(std::errc::invalid_argument), "test");
    const Error copy(original);
    EXPECT_EQ(copy.value(), original.value());
    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.context(), original.context());
}

TEST(ErrorTest, MoveConstruction) {
    Error original(std::make_error_code(std::errc::invalid_argument), "test");
    const Error moved(std::move(original));
    EXPECT_EQ(moved.value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(moved.message(), "test: Invalid argument");
    EXPECT_TRUE(original.context().empty());
}

TEST(ErrorTest, CopyAssignment) {
    const Error original(std::make_error_code(std::errc::invalid_argument), "test");
    Error copy(std::make_error_code(std::errc::permission_denied), "other");
    copy = original;
    EXPECT_EQ(copy.value(), original.value());
    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.context(), original.context());
}

TEST(ErrorTest, MoveAssignment) {
    Error original(std::make_error_code(std::errc::invalid_argument), "test");
    Error other(std::make_error_code(std::errc::permission_denied), "other");
    other = std::move(original);
    EXPECT_EQ(other.value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(other.message(), "test: Invalid argument");
    EXPECT_TRUE(original.context().empty());
}

TEST(ErrorTest, SelfAssignment) {
    Error error(std::make_error_code(std::errc::invalid_argument), "test");
    error = error;
    EXPECT_EQ(error.value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(error.message(), "test: Invalid argument");
}

// Test for Error class with error code and context
TEST(ErrorTest, ConstructionWithErrorCodeAndContext) {
    const Error error(std::make_error_code(std::errc::invalid_argument), "Invalid argument provided");
    EXPECT_TRUE(error);
    EXPECT_EQ(error.value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(error.message(), "Invalid argument provided: Invalid argument");
}

// Test for Error class construction with ExtraError code
TEST(ErrorTest, ConstructionWithExtraError) {
    const Error error(ExtraError::bad_alloc, "Allocation failed");
    EXPECT_TRUE(error);
    EXPECT_EQ(error.value(), static_cast<int>(ExtraError::bad_alloc));
    EXPECT_EQ(error.message(), "Allocation failed: Bad allocation exception");
}

// Test for Error class with empty context
TEST(ErrorTest, ConstructionWithEmptyContext) {
    const Error error(std::make_error_code(std::errc::invalid_argument), "");
    EXPECT_TRUE(error);
    EXPECT_EQ(error.value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(error.message(), "Invalid argument");
}

// Test for Error class with unknown error code
TEST(ErrorTest, ConstructionWithUnknownErrorCode) {
    const Error error(static_cast<ExtraError>(999), "Unknown error");
    EXPECT_TRUE(error);
    EXPECT_EQ(error.value(), 999);
    EXPECT_EQ(error.message(), "Unknown error: Unrecognized ExtraError");
}

// Test for Error class with null error code
TEST(ErrorTest, ConstructionWithNullErrorCode) {
    const Error error(std::error_code(), "No error");
    EXPECT_FALSE(error);
    EXPECT_EQ(error.value(), 0);
    // EXPECT_EQ(error.message(), "No error: Success"); // the message is platform-dependent
}

// Test for Error comparison
TEST(ErrorTest, Comparison) {
    const Error error1(std::make_error_code(std::errc::invalid_argument));
    const Error error2(std::make_error_code(std::errc::invalid_argument));
    const Error error3(std::make_error_code(std::errc::result_out_of_range));

    EXPECT_EQ(error1, error2);
    EXPECT_NE(error1, error3);

    const Error error4(ExtraError::bad_alloc, "Allocation failed");
    const Error error5(ExtraError::bad_alloc, "Another allocation failed"); // Different context
    const Error error6(ExtraError::bad_cast);

    EXPECT_EQ(error4, error5); // Should compare equal based on error code only
    EXPECT_NE(error4, error6);
}

TEST(ErrorTest, ComparisonWithCondition) {
    const Error error(std::make_error_code(std::errc::invalid_argument));
    EXPECT_TRUE(error == std::errc::invalid_argument);
    EXPECT_FALSE(error == std::errc::permission_denied);
}

TEST(ErrorTest, ComparisonOperatorOrder) {
    const Error error1(std::make_error_code(std::errc::permission_denied));
    const Error error2(std::make_error_code(std::errc::invalid_argument));
    EXPECT_TRUE(error1 < error2);
    EXPECT_FALSE(error2 < error1);
    EXPECT_TRUE(error2 > error1);
    EXPECT_FALSE(error1 > error2);
}

// Test operator=
TEST(ErrorTest, ErrorStreamOutput) {
    const Error error(std::make_error_code(std::errc::invalid_argument), "Test context");
    std::stringstream ss;
    ss << error;
    EXPECT_THAT(ss.str(), ::testing::HasSubstr("Test context: Invalid argument"));
    EXPECT_THAT(ss.str(), ::testing::HasSubstr("error_code: 22"));
    EXPECT_THAT(ss.str(), ::testing::HasSubstr("generic category"));
}

// Test is() method
TEST(ErrorTest, IsSpecificError) {
    const Error error(std::make_error_code(std::errc::invalid_argument), "Test context");
    EXPECT_TRUE(error.is(std::errc::invalid_argument));
    EXPECT_FALSE(error.is(std::errc::result_out_of_range));
}

// Test is() specialization
TEST(ErrorTest, IsSpecificExtraError) {
    const Error error(ExtraError::bad_alloc, "Test context");
    EXPECT_TRUE(error.is(ExtraError::bad_alloc));
    EXPECT_FALSE(error.is(ExtraError::bad_cast));
}

// Test is_any_of()
TEST(ErrorTest, IsAnyOf) {
    const Error error(std::make_error_code(std::errc::invalid_argument));
    EXPECT_TRUE(error.is_any_of(std::errc::invalid_argument, std::errc::result_out_of_range));
    EXPECT_FALSE(error.is_any_of(std::errc::result_out_of_range, std::errc::operation_not_permitted));
}

// Test is_any_of() specialization
TEST(ErrorTest, IsAnyOfExtraError) {
    const Error error(ExtraError::bad_alloc);
    EXPECT_TRUE(error.is_any_of(ExtraError::bad_alloc, ExtraError::bad_cast));
    EXPECT_FALSE(error.is_any_of(ExtraError::bad_cast, ExtraError::bad_function_call));
}

// Test swap() friend function
TEST(ErrorTest, Swap) {
    Error error1(std::make_error_code(std::errc::invalid_argument), "Error 1");
    Error error2(std::make_error_code(std::errc::result_out_of_range), "Error 2");

    swap(error1, error2);

    EXPECT_EQ(error1.value(), static_cast<int>(std::errc::result_out_of_range));
    EXPECT_EQ(error1.context(), "Error 2");
    EXPECT_EQ(error2.value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(error2.context(), "Error 1");
}

// ///////////////////////// Tests on utility functions //////////////////////////////

// Test for make_error utility
TEST(MakeErrorTest, CreateError) {
    auto result = make_error<int>(std::errc::invalid_argument, "Invalid argument");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(result.error().message(), "Invalid argument: Invalid argument");
}

// make_error() should work with ExtraError
TEST(MakeErrorTest, CreateExtraError) {
    auto result = make_error<int>(ExtraError::bad_alloc, "Allocation failed");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_alloc));
    EXPECT_EQ(result.error().message(), "Allocation failed: Bad allocation exception");
}

// Test for make_error with empty context
TEST(MakeErrorTest, CreateErrorWithEmptyContext) {
    auto result = make_error<int>(std::errc::invalid_argument, "");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(result.error().message(), "Invalid argument");
}

// make_error() should map regex errors
TEST(MakeErrorTest, RegexErrorMapping) {
    auto result = make_error<void>(std::regex_constants::error_brack, "Mismatched brackets");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(result.error().message(),
              "Mismatched brackets: Regex error: mismatched square brackets ('[' and ']'): Invalid argument");
}

// Test for make_error with an unknown regex error
TEST(MakeErrorTest, UnknownRegexErrorMapping) {
    auto result = make_error<void>(static_cast<std::regex_constants::error_type>(999), "Unknown regex error");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::unknown_error));
    EXPECT_EQ(result.error().message(), "Unknown regex error: Regex error: unknown error: Unknown error");
}

TEST(MakeErrorFromErrnoTest, CreateErrorFromErrno) {
    errno = EPERM;
    auto result = make_error_from_errno<int>("Operation not permitted");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), EPERM);
    EXPECT_EQ(result.error().message(), "Operation not permitted: Operation not permitted");
    EXPECT_EQ(errno, 0); // Ensure errno is reset
}

TEST(MakeErrorTest, CreateErrorWithLongContext) {
    std::string long_context(1000, 'a');
    auto result = make_error<int>(std::errc::invalid_argument, long_context);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().context().length(), 1000);
}

TEST(MakeErrorTest, CreateErrorWithComplexErrorCode) {
    const auto complex_error = std::make_error_code(std::io_errc::stream);
    auto result = make_error<void>(complex_error, "IO Error");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().error_code(), complex_error);
}

// Test for try_catch utility with no exception
TEST(TryCatchTest, NoException) {
    const auto result = try_catch([] { return 42; });
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

// Test for try_catch utility with std::invalid_argument
TEST(TryCatchTest, InvalidArgumentException) {
    auto result = try_catch([]() -> int {
        throw std::invalid_argument("Invalid argument");
    }, "TryCatchTest");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::invalid_argument));
    EXPECT_EQ(result.error().message(), "TryCatchTest: Invalid argument: Invalid argument exception");
}

// Test for try_catch with no context
TEST(TryCatchTest, NoContext) {
    auto result = try_catch([]() -> int { throw std::invalid_argument("Invalid argument"); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::invalid_argument));
    EXPECT_EQ(result.error().message(), "Invalid argument: Invalid argument exception");
}

// Test for try_catch utility with an unknown exception
TEST(TryCatchTest, UnknownException) {
    auto result = try_catch([]() -> int { throw 42; });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::unknown_exception));
    EXPECT_EQ(result.error().message(), "Unknown exception: Unknown exception caught");
}

TEST(TryCatchTest, DomainErrorException) {
    auto result = try_catch([]() -> int { throw std::domain_error("Domain error"); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::argument_out_of_domain));
    EXPECT_EQ(result.error().message(), "Domain error: Numerical argument out of domain");
}

TEST(TryCatchTest, OutOfRangeException) {
    auto result = try_catch([]() -> int { throw std::out_of_range("Out of range"); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::result_out_of_range));
    // EXPECT_EQ(result.error().message(), "Out of range: Numerical result out of range"); // platform-dependent
}

TEST(TryCatchTest, FutureErrorException) {
    auto result = try_catch([]() -> int { throw std::future_error(std::future_errc::broken_promise); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::future_errc::broken_promise));
    EXPECT_EQ(result.error().message(), "std::future_error: Broken promise: Broken promise");
}

TEST(TryCatchTest, RangeErrorException) {
    auto result = try_catch([]() -> int { throw std::range_error("Range error"); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::result_out_of_range));
    // EXPECT_EQ(result.error().message(), "Range error: Numerical result out of range"); // platform-dependent
}

TEST(TryCatchTest, OverflowErrorException) {
    auto result = try_catch([]() -> int { throw std::overflow_error("Overflow error"); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::value_too_large));
    EXPECT_EQ(result.error().message(), "Overflow error: Value too large for defined data type"); // platform-dependent
}

TEST(TryCatchTest, UnderflowErrorException) {
    auto result = try_catch([]() -> int { throw std::underflow_error("Underflow error"); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::value_too_small));
    EXPECT_EQ(result.error().message(), "Underflow error: Value too small (underflow exception)");
}

TEST(TryCatchTest, RegexErrorException) {
    auto result = try_catch([]() -> int {
        throw std::regex_error(std::regex_constants::error_brack);
    }, "Regex error");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::invalid_argument));
    auto &&msg = result.error().message();
    EXPECT_TRUE(msg.contains("Regex error") && msg.contains('[') && msg.contains(']'));
}

TEST(TryCatchTest, SystemErrorException) {
    auto result = try_catch(
        []() -> int { throw std::system_error(std::make_error_code(std::errc::permission_denied)); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::permission_denied));
    EXPECT_EQ(result.error().message(), "Permission denied");
}

TEST(TryCatchTest, NonExistentLocalTimeException) {
    auto result = try_catch([]() -> int {
        constexpr std::chrono::local_time<std::chrono::seconds> tmp{};
        const std::chrono::local_info info{};
        throw std::chrono::nonexistent_local_time(tmp, info);
    }, "Nonexistent local time");

    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::nonexistent_local_time));
    auto &&msg = result.error().message();
    EXPECT_TRUE(msg.contains("Nonexistent local time") && msg.contains("1970-01-01 00:00:00"));
}

TEST(TryCatchTest, AmbiguousLocalTimeException) {
    auto result = try_catch([]() -> int {
        constexpr std::chrono::local_time<std::chrono::seconds> tmp{};
        const std::chrono::local_info info{};
        throw std::chrono::ambiguous_local_time(tmp, info);
    }, "Ambiguous local time");

    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::ambiguous_local_time));
    auto &&msg = result.error().message();
    EXPECT_TRUE(msg.contains("Ambiguous local time exception") && msg.contains("1970-01-01 00:00:00 is ambiguous"));
}

TEST(TryCatchTest, FormatErrorException) {
    auto result = try_catch([]() -> int { throw std::format_error("Format error"); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::format_error));
    EXPECT_EQ(result.error().message(), "Format error: Format error exception");
}

TEST(TryCatchTest, BadAllocException) {
    auto result = try_catch([]() -> int { throw std::bad_alloc(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_alloc));
    EXPECT_EQ(result.error().message(), "std::bad_alloc: Bad allocation exception");
}

TEST(TryCatchTest, BadTypeidException) {
    auto result = try_catch([]() -> int { throw std::bad_typeid(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_typeid));
    EXPECT_EQ(result.error().message(), "std::bad_typeid: Bad typeid exception");
}

TEST(TryCatchTest, BadCastException) {
    auto result = try_catch([]() -> int { throw std::bad_cast(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_cast));
    EXPECT_EQ(result.error().message(), "std::bad_cast: Bad cast exception");
}

TEST(TryCatchTest, BadExpectedAccessException) {
    auto result = try_catch([]() -> int { throw std::bad_expected_access<int>(6); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_expected_access));
    EXPECT_EQ(result.error().message(),
              "bad access to std::expected without expected value: Bad expected access exception");
}

// Note: The following two tests exit with bus error on macOS
#ifndef  __APPLE__
TEST(TryCatchTest, BadVariantAccessException) {
    auto result = try_catch([]() -> int { throw std::bad_variant_access(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_variant_access));
    EXPECT_EQ(result.error().message(), "bad variant access: Bad variant access exception");
}

TEST(TryCatchTest, BadOptionalAccessException) {
    auto result = try_catch([]() -> int { throw std::bad_optional_access(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_optional_access));
    EXPECT_EQ(result.error().message(), "bad optional access: Bad optional access exception");
}
#endif

TEST(TryCatchTest, BadWeakPtrException) {
    auto result = try_catch([]() -> int { throw std::bad_weak_ptr(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_weak_ptr));
    EXPECT_EQ(result.error().message(), "bad_weak_ptr: Bad weak pointer exception");
}

TEST(TryCatchTest, BadFunctionCallException) {
    auto result = try_catch([]() -> int { throw std::bad_function_call(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_function_call));
    EXPECT_EQ(result.error().message(), "bad_function_call: Bad function call exception");
}

TEST(TryCatchTest, BadException) {
    auto result = try_catch([]() -> int { throw std::bad_exception(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::bad_exception));
    EXPECT_EQ(result.error().message(), "std::bad_exception: Bad exception");
}

TEST(TryCatchTest, GenericException) {
    auto result = try_catch([]() -> int { throw std::exception(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(ExtraError::exception));
    EXPECT_EQ(result.error().message(), "std::exception: Exception caught");
}

TEST(TryCatchTest, NestedExceptions) {
    auto result = try_catch([]() -> int {
        try {
            throw std::invalid_argument("Inner");
        } catch (...) {
            throw std::runtime_error("Outer");
        }
    });
    EXPECT_FALSE(result);
    EXPECT_TRUE(result.error().message().find("Outer") != std::string::npos);
}

TEST(TryCatchTest, VoidReturnType) {
    const auto result = try_catch([] -> VoidResult {
        /* do nothing */
        return {};
    });
    EXPECT_TRUE(result);
}

// Test for with_errno utility
TEST(WithErrnoTest, NoError) {
    const auto result = with_errno([] { return 42; });
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

TEST(WithErrnoTest, ErrorOccurred) {
    auto result = with_errno([]() -> int {
        errno = EINVAL;
        return -1;
    }, "System call failed");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), EINVAL);
    EXPECT_EQ(result.error().message(), "System call failed: Invalid argument");
}

TEST(WithErrnoTest, VoidReturnTypeNoError) {
    const auto result = with_errno([] {
        /* No-op */
    });
    EXPECT_TRUE(result);
}

TEST(WithErrnoTest, VoidReturnTypeError) {
    auto result = with_errno([] { errno = EACCES; }, "Operation failed");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), EACCES);
    EXPECT_EQ(result.error().message(), "Operation failed: Permission denied");
}

// Test for with_errno with no error and empty context
TEST(WithErrnoTest, NoErrorEmptyContext) {
    const auto result = with_errno([] { return 42; }, "");
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

// Test for invoke_with_syscall_api utility
TEST(InvokeWithSyscallApiTest, NoError) {
    const auto result = invoke_with_syscall_api([]() noexcept { return 42; });
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

// Test for invoke_with_syscall_api with no error and empty context
TEST(InvokeWithSyscallApiTest, NoErrorEmptyContext) {
    const auto result = invoke_with_syscall_api([]() noexcept { return 42; }, "");
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

TEST(InvokeWithSyscallApiTest, ErrorOccurred) {
    auto result = invoke_with_syscall_api([]() noexcept -> int {
        errno = EINVAL;
        return -1;
    }, "Syscall failed");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), EINVAL);
    EXPECT_EQ(result.error().message(), "Syscall failed: Invalid argument");
}

TEST(InvokeWithSyscallApiTest, NegativeOneNoError) {
    const auto result = invoke_with_syscall_api([]() noexcept { return 0; });
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 0);
}

TEST(FirstOfTest, FirstSuccess) {
    auto result1 = make_error<int>(std::errc::invalid_argument, "First error");
    auto result2 = Result<int>(42);
    auto result3 = make_error<int>(std::errc::permission_denied, "Third error");

    const auto combined = first_of({result1, result2, result3});
    EXPECT_TRUE(combined);
    EXPECT_EQ(combined.value(), 42);
}

TEST(FirstOfTest, AllErrors) {
    auto result1 = make_error<int>(std::errc::invalid_argument, "First error");
    auto result2 = make_error<int>(std::errc::permission_denied, "Second error");
    auto result3 = make_error<int>(std::errc::operation_canceled, "Third error");

    auto combined = first_of({result1, result2, result3});
    EXPECT_FALSE(combined);
    EXPECT_EQ(combined.error().message(),
              "First error: Invalid argument; Second error: Permission denied; Third error: "
              "Operation canceled: Unknown error");
    EXPECT_EQ(combined.error().category().name(), "ExtraError");
}

// Test for first_of with all successes
TEST(FirstOfTest, AllSuccesses) {
    auto result1 = Result<int>(1);
    auto result2 = Result<int>(2);
    auto result3 = Result<int>(3);

    const auto combined = first_of({result1, result2, result3});
    EXPECT_TRUE(combined);
    EXPECT_EQ(combined.value(), 1);
}

TEST(FirstOfTest, EmptyList) {
    const auto result = first_of<int>({});
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().message(), "No alternatives provided: Invalid argument");
}

TEST(FirstOfTest, SingleError) {
    auto result = first_of({make_error<int>(std::errc::invalid_argument, "Single error")});
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().message(), "Single error: Invalid argument: Unknown error");
}

TEST(FirstOfTest, SingleSuccess) {
    const auto result = first_of({Result<int>(42)});
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

TEST(StdFormatTest, ErrorFormat) {
    Error err(std::make_error_code(std::errc::invalid_argument), "test error");
    const std::string formatted = std::format("Error: {}", err);
    EXPECT_THAT(formatted, ::testing::HasSubstr("test error: Invalid argument"));
    EXPECT_THAT(formatted, ::testing::HasSubstr("error_code: 22"));
    EXPECT_THAT(formatted, ::testing::HasSubstr("category: generic"));
}

TEST(StdHashTest, ErrorHash) {
    const Error err1(std::make_error_code(std::errc::invalid_argument), "test error 1");
    const Error err2(std::make_error_code(std::errc::invalid_argument), "test error 2");
    const Error err3(std::make_error_code(std::errc::permission_denied), "test error 3");

    constexpr std::hash<Error> hasher;
    EXPECT_EQ(hasher(err1), hasher(err2));
    EXPECT_NE(hasher(err1), hasher(err3));
}
