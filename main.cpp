#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <utility>
#include <mdspan>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <variant>
#include <print>
#include <format>
#include <leviathan/math/vector.hpp>
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

template <std::meta::info Info>
struct metainfo
{
    constexpr static bool is_template_function = std::meta::is_function_template(Info);
    constexpr static bool is_function = std::meta::is_function(Info);

    static_assert(is_template_function, "Info must be a function and function template");

    static constexpr std::string_view display_name()
    {
        return display_string_of(Info);
    }

    static constexpr std::vector<std::string> template_parameter_names()
    {
        std::vector<std::string> names;

        // template for (constexpr auto param : define_static_array(parameters_of(Info)))
        // {
        //     names.emplace_back(display_string_of(param));
        // }
        return names;
    }

};

template <typename T>
void test1() { }

template<typename T>
constexpr T add(T a, T b) {
    return a + b ;
}

int main() {
    constexpr auto add_tmpl = ^^add;
    // 不写 <...>，直接传调用参数，自动推导 T=int
    constexpr auto res = template [:add_tmpl:]<double>;
}




