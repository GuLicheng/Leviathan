#include <leviathan/nom/nom.hpp>
#include <leviathan/meta/type.hpp>
#include <print>
#include <leviathan/type_caster.hpp>
#include <leviathan/extc++/ranges.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>

struct Color
{
    int r, g, b;
};

int main(int argc, char const *argv[])
{
    Color color = std::make_from_tuple<Color>(std::tuple{255, 0, 255});

    std::string x; std::cin >> x;

    std::println("{}", cpp::cast<double>(x));

    return 0;
}
