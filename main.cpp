#include <iostream>
#include <print>
#include <leviathan/extc++/all.hpp>
#include <nom/all.hpp>

using nom::error;
using nom::iresult;

enum class [[=cpp::derive::debug]] ErrorCode
{
    TestError = 1001,
};

auto ReplaceErrorCode = [](nom::error<ErrorCode> e) -> int
{
    return 1;
};

int main()
{
    using Result1 = iresult<int, std::string_view, ErrorCode>;
    using Result2 = iresult<int, std::string_view, int>;

    auto r1 = Result1::make_err(nom::error<ErrorCode>::make_recoverable(ErrorCode::TestError));
    auto r2 = r1.map_err(ReplaceErrorCode);

    std::print("ex: {}\n", r1);
    std::print("ex: {}\n", r2);

    return 0;
}