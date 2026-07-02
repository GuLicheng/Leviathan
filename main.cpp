#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>

struct [[=cpp::derive::debug, =cpp::derive::from<cpp::json::value>]] FlattenValue
{
    int A;
    int B;
};

struct [[=cpp::refl::lowercase, =cpp::derive::from<cpp::json::value>, =cpp::derive::debug]] Foo
{
    [[=cpp::refl::guard([](int x) { return x >= 0; })]] 
    [[=cpp::refl::choice(0, 1, 2, 3, 4, 5)]]
    [[=cpp::refl::default_value(10)]]
    int X;

    [[=cpp::refl::flatten]]
    FlattenValue Y;
};


int main(int argc, char const *argv[])
{

    cpp::json::value v = {
        { "x", 3 },
        { "A", 1 },
        { "B", 2 }
    };

    auto foo = cpp::cast<Foo>(v);

    std::println("{}", foo);

    return 0;
}



