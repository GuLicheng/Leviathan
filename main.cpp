#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <assert.h>

struct A
{
    int x;
    double y;

    [[=cpp::refl::alias("Hello", "World")]]
    [[=cpp::refl::uppercase]]
    std::string z;
};

std::vector<std::string> extract_field_names()
{
    return cpp::refl::handle<^^A::z>::identifier_and_aliases();
}

constexpr void print()
{
    auto vec = extract_field_names();
    // std::println("{}", vec | cpp::action::for_each(cpp::views::format));
    std::println("{}", vec | cpp::action::for_each(cpp::views::format));
}

int main()
{
    // template for (constexpr auto name : 
    //     define_static_array(cpp::refl::all_bases_of( ^^std::tuple<int, double> /* , std::meta::access_context::unchecked() */)))
    // {
    //     std::println("{}", display_string_of(name));
    // }

    print();

}