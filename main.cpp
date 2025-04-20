#include <leviathan/extc++/all.hpp>
#include <algorithm>
#include <leviathan/print.hpp>
#include <leviathan/stopwatch.hpp>
#include <leviathan/timer.hpp>
#include <leviathan/config_parser/cmd/command.hpp>
#include <thread>
#include <leviathan/callable.hpp>

struct Lambda
{
    void operator()() { }
};

int main(int argc, char const *argv[])
{
    cpp::callable calls;

    calls.register_handler("test", [](int a, int b) { return a + b; });

    auto res = calls.call<int>("test", 1, 2);

    Console::WriteLine("Result: {}", res);

    using T = typename cpp::meta::function_traits<Lambda>::class_type;

    return 0;
}
