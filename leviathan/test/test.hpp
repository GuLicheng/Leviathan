/*

    - Terminal output coloring and formatting

    - Logging and debugging utilities

    - Global state management for tests

*/
#pragma once

#include <format>
#include <vector>
#include <source_location>

namespace cpp::test
{

// Some annotations
inline constexpr struct { } test_function;


class recorder
{
    std::vector<std::string> logs;

public:

    recorder() = default;

    void log(std::string message) 
    {
        logs.emplace_back(std::move(message));
    }
};

inline recorder& get_recorder()
{
    static recorder instance;
    return instance;
}

template <typename Expected, typename Actual>
void assert_equal(const Expected& expected, const Actual& actual, std::source_location location = std::source_location::current())
{
    if (expected != actual)
    {
        std::string message = std::format("Assertion failed at {}:{}: Expected '{}', but got '{}'", location.file_name(), location.line(), expected, actual);
        get_recorder().log(message);
    }
}

template <typename Expression>
void assert_true(const Expression& expression, std::source_location location = std::source_location::current())
{
    if (!expression)
    {
        std::string message = std::format("Assertion failed at {}:{}: Expression is not true", location.file_name(), location.line());
        get_recorder().log(message);
    }
}

}  // namespace cpp::test