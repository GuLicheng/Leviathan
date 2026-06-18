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

template <typename T>
struct type_handle
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

struct Base
{
    int X = 1;
};

inline constexpr auto LowerCase [[=cpp::refl::rename_annotation]] = [](std::string name) {
    return name | std::views::transform([](char c) { return std::tolower(c); }) | std::ranges::to<std::string>();
} ;

struct [[=cpp::derive::into<cpp::json::value>, =cpp::derive::debug]] Derive : public Base
{
    [[=cpp::refl::rename("y_value")]]
    double Y = 3.14;

    [[=LowerCase]]
    double Z = 2.17;
    double W = 0.0;
};

class [[=cpp::derive::tuple_like]] Point { int X; int Y; };


int main(int argc, char const *argv[])
{
    type_handle<std::vector<int>>::show_all_members();
    type_handle<Point>::show_all_members();
    type_handle<std::tuple<int, int>>::show_all_members();

    auto j = cpp::json::make(Derive{1, 3.14});

    Derive d;

    template for (constexpr auto anno : define_static_array(annotations_of(^^Derive::Z)))
    {
        constexpr auto instance = std::meta::extract<typename [:type_of(anno):]>(anno);
        constexpr auto msg = std::meta::annotations_of(^^instance).size();
        std::print("Annotation: {}\n", msg);
    }

    std::println("{}", j);

    return 0;
}

