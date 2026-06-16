#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <utility>
#include <mdspan>
#include <iostream>
#include <variant>
#include <print>
#include <format>
#include <tuple>



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


    return 0;
}

