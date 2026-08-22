#include <iostream>
#include <print>
#include <leviathan/extc++/all.hpp>
#include <nom/all.hpp>

using nom::err;

int main()
{
    // 测试1：Incomplete 需要至少16字节
    auto e1 = err<int>::make_incomplete(16);
    std::println("e1 need bytes: {}", e1);

    // 测试2：Error(可回溯错误)，负载为1001
    auto e2 = err<int>::make_error(1001);
    std::println("e2 error: {}", e2);

    // 测试3：Failure(不可回溯)，负载9999
    auto e3 = err<int>::make_failure(9999);
    std::println("e3 failure: {}", e3);

    // 测试拷贝构造
    auto e4 = e2;
    std::println("e4 error: {}", e4);

    return 0;
}
