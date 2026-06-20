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
#include <python3.14/Python.h>
#include <tuple>

template <typename T>
struct type
{
    static constexpr void show_all_members()
    {
        constexpr static auto members = define_static_array(cpp::refl::all_nsdm_unchecked<T>());
        template for (constexpr auto member : members)
        {
            std::print("Member: {}\n", has_identifier(member) ? identifier_of(member) : "<unnamed>");
        }
    }
};


int main(int argc, char const *argv[])
{

    return 0;
}

