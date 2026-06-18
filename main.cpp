#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <utility>
#include <mdspan>
#include <iostream>
#include <variant>
#include <print>
#include <format>
#include <tuple>

struct Base
{
    int X = 1;
};

struct [[=cpp::derive::into<cpp::json::value>]] Derive : public Base
{
    double Y = 3.14;
    double Z = 2.17;
    double W = 0.0;
};

template <typename T>
constexpr void show_all_members()
{
    constexpr static auto members = define_static_array(cpp::refl::all_nsdm_unchecked<T>());
    template for (constexpr auto member : members)
    {
        std::print("Member: {}\n", has_identifier(member) ? identifier_of(member) : "<unnamed>");
    }
}

class [[=cpp::derive::tuple_like]] Point { int X; int Y; };

int main(int argc, char const *argv[])
{
    show_all_members<std::vector<int>>();
    show_all_members<Point>();
    show_all_members<std::tuple<int, int>>();

    auto j = cpp::json::make(Derive{1, 3.14});

    std::println("{}", j);

    return 0;
}

