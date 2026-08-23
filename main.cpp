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

auto ReplaceErrorCode = [](int code) 
{
    return ErrorCode(code);
};

int main()
{
    // 测试1：Incomplete 需要至少16字节
    auto e1 = error<int>::make_incomplete(16);
    std::println("e1 need bytes: {}", e1);

    // 测试2：Error(可回溯错误)，负载为1001
    auto e2 = error<int>::make_recoverable(1001);
    std::println("e2 error: {}", e2);

    // 测试3：Failure(不可回溯)，负载9999
    auto e3 = error<int>::make_unrecoverable(9999);
    std::println("e3 failure: {}", e3);

    // 测试拷贝构造
    auto e4 = e2;
    std::println("e4 error: {}", e4);

    using IR = iresult<std::string_view, int, int>;
    using ErrTy = error<int>;

    auto ir_ok = IR::make_ok(std::pair{std::string_view("remain"), 42});
    auto ir_inc = IR::make_err(ErrTy::make_incomplete(16));
    auto ir_err = IR::make_err(ErrTy::make_recoverable(701));
    auto ir_fail = IR::make_err(ErrTy::make_unrecoverable(801));

    if (ir_ok.is_ok())
    {
        auto& [rest, out] = ir_ok.unwrap_ok();
        std::cout << "iresult ok: rest=" << rest << ", output=" << out << "\n";
    }

    if (ir_inc.is_err())
    {
        auto& inner_err = ir_inc.unwrap_err();
        std::cout << "iresult incomplete: need=" << inner_err.as_incomplete().value << "\n";
    }
    if (ir_err.is_err())
    {
        auto& inner_err = ir_err.unwrap_err();
        std::cout << "iresult error: val=" << inner_err.as_recoverable().value << "\n";
    }
    if (ir_fail.is_err())
    {
        auto& inner_err = ir_fail.unwrap_err();
        std::cout << "iresult failure: val=" << inner_err.as_unrecoverable().value << "\n";
    }

    auto ir_err2 = ir_err.map_err(ReplaceErrorCode);
    std::println("ir_err2: {}", ir_err2);

    return 0;
}