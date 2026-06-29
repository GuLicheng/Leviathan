#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <print>

struct Foo
{
    [[=cpp::refl::guard([](int x) { return x >= 0; })]] int X;
};

int main(int argc, char const *argv[])
{
    std::println("Default value of Foo::X is {}", cpp::refl::check_field(Foo{420}));

    return 0;
}
