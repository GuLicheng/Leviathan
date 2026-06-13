#pragma once

#include <leviathan/extc++/format.hpp>
#include <variant>

namespace cpp
{

struct tag_union_formatter
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    { 
        return ctx.begin();
    }

    template <typename Union, typename FmtContext>
    typename FmtContext::iterator format(const Union& u, FmtContext& ctx) const
    {
        return std::visit([&ctx](const auto& value) {
            return std::format_to(ctx.out(), "{}", value);
        }, u);
    }
};

}

template <typename... Types>
struct std::formatter<std::variant<Types...>> : cpp::tag_union_formatter { };

template <>
struct std::formatter<std::monostate> : cpp::universal_formatter { };


template <typename T>
void ShowName()
{
    std::cout << display_string_of(^^T) << std::endl;
}


template <typename List, typename T> struct ListAppend;

template <template <typename...> typename Template, typename... Ts, typename T>
struct ListAppend<Template<Ts...>, T> {
    using type = std::conditional_t<(((std::same_as<Ts, T>) || ...)), Template<Ts...>, Template<Ts..., T>>;
};

// template <size_t N> struct Undefined;

// // Class ^^Undefined
// template <std::meta::info Class>
// struct VariantBuilder {

//     static consteval bool is_defined(size_t index) {
//         return is_complete_type(substitute(Class, {std::meta::reflect_constant(index)}));
//     }

//     static consteval size_t get_last_index() {
//         size_t k = 0;
//         for (; is_defined(k); ++k);
//         return k;
//     }

//     template <typename T>   
//     static consteval void put()
//     {
//         constexpr auto index = get_last_index();
//         define_aggregate(
//             substitute(Class, { std::meta::reflect_constant(index) }),
//             { std::meta::data_member_spec(^^T, {.name = "value"}) }
//         );
//     }

//     template <size_t Index = get_last_index() - 1>
//     using get_t = decltype(Undefined<Index>::value);

//     template <typename T>
//     static consteval void update_variant()
//     {
//         using CurrentType = get_t<>;
//         using NextType = typename ListAppend<CurrentType, T>::type;
//         put<NextType>();
//     }

// };

// using Builder = VariantBuilder<^^Undefined>;

// consteval {
//     Builder::put<std::variant<std::monostate>>(); 
//     Builder::update_variant<double>();
//     Builder::update_variant<bool>();
// }

// struct Foo {
//     int a;
//     double b;
// };

// enum class Color { Red, Green, Blue };

// consteval {
//     Builder::update_variant<Foo>();
//     Builder::update_variant<Color>();
// }
