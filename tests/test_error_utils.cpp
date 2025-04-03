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
    EXPECT_EQ(error.message(), "Success");
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

// Test for make_error utility
TEST(MakeErrorTest, CreateError) {
    auto result = make_error<int>(std::errc::invalid_argument, "Invalid argument");
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(std::errc::invalid_argument));
    EXPECT_EQ(result.error().message(), "Invalid argument: Invalid argument");
}

// Test for try_catch utility with no exception
TEST(TryCatchTest, NoException) {
    const auto result = try_catch([] { return 42; });
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

// Test for try_catch utility with std::invalid_argument
TEST(TryCatchTest, InvalidArgumentException) {
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

// Test for invoke_with_syscall_api utility
TEST(InvokeWithSyscallApiTest, NoError) {
    const auto result = invoke_with_syscall_api([]() noexcept { return 42; });
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
