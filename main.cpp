#include <iostream>
#include <print>
#include <leviathan/extc++/all.hpp>
#include <nom/all.hpp>

using nom::error;
using nom::iresult;

enum class ErrorCode
{
    TestError = 1001,
};

auto ReplaceErrorCode = [](nom::error<int> e) -> nom::error<ErrorCode>
{
    return nom::error<ErrorCode>::make_recoverable(ErrorCode::TestError);
};

int main()
{
    using Result1 = iresult<int, std::string_view, ErrorCode>;
    using Result2 = iresult<int, std::string_view, int>;

    // auto r1 = Result1::make_err(nom::error<ErrorCode>::make_recoverable(ErrorCode::TestError));
    // auto r2 = r1.map_err(ReplaceErrorCode);

    std::expected<int, std::string> ex = std::unexpected("error");

    std::print("ex: {}\n", ex);

    return 0;
}