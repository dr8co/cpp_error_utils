#include <error_utils.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace error_utils;

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

// Test for Error comparison
TEST(ErrorTest, Comparison) {
    const Error error1(std::make_error_code(std::errc::invalid_argument));
    const Error error2(std::make_error_code(std::errc::invalid_argument));
    const Error error3(std::make_error_code(std::errc::result_out_of_range));

    EXPECT_EQ(error1, error2);
    EXPECT_NE(error1, error3);
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


