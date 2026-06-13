#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <utility>
#include <mdspan>
#include <iostream>
#include <variant>
#include <print>
#include <tuple>

template <typename T>
void ShowName()
{
    std::cout << display_string_of(^^T) << std::endl;
}



using Builder = cpp::variant_builder;

consteval {
    Builder::declare<double>();
    Builder::declare<bool>();
}

struct Foo {
    int a;
    double b;
};

enum class Color { Red, Green, Blue };

consteval {
    Builder::declare<Foo>();
    Builder::declare<Color>();
    Builder::declare<std::tuple<int, double, std::string>>();
}

int main(int argc, char const *argv[])
{
    using T = Builder::get_t<>;
    std::println("Variant type: {}", (display_string_of(^^T)));
    return 0;
}

