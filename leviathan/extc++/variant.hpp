#pragma once

#include <leviathan/extc++/format.hpp>
#include <variant>

namespace cpp
{

struct tag_union_formatter
{
    template <typename ParseContext>
    static constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    { 
        return ctx.begin();
    }

    template <typename Variant, typename FmtContext>
    static typename FmtContext::iterator format(const Variant& u, FmtContext& ctx)
    {
        return std::visit([&ctx](const auto& value) {
            return std::format_to(ctx.out(), "{}", value);
        }, u);
    }
};

namespace detail
{

template <typename List, typename T> struct type_list_append;

template <template <typename...> typename Template, typename... Ts, typename T>
struct type_list_append<Template<Ts...>, T> {
    using type = std::conditional_t<(((std::same_as<Ts, T>) || ...)), Template<Ts...>, Template<Ts..., T>>;
};   
    
template <size_t N> struct undefined;

// Class ^^Undefined
template <std::meta::info Class>
struct variant_builder 
{
    static consteval bool is_defined(size_t index) {
        return is_complete_type(substitute(Class, {std::meta::reflect_constant(index)}));
    }

    static consteval size_t get_last_index() {
        size_t k = 0;
        for (; is_defined(k); ++k);
        return k;
    }

    template <typename T>   
    static consteval void put()
    {
        constexpr auto index = get_last_index();
        define_aggregate(
            substitute(Class, { std::meta::reflect_constant(index) }),
            { std::meta::data_member_spec(^^T, { .name = "value" }) }
        );
    }

    template <size_t Index = get_last_index() - 1>
    using get_t = decltype(undefined<Index>::value);

    template <typename T>
    static consteval void declare()
    {
        using CurrentType = get_t<>;
        using NextType = typename type_list_append<CurrentType, T>::type;
        put<NextType>();
    }
};

} // namespace detail

using variant_builder = detail::variant_builder<^^detail::undefined>;

consteval { variant_builder::put<std::variant<std::monostate>>(); }

/*
consteval {
    Builder::declare<double>();
    Builder::declare<bool>();
}
*/

}  // namespace cpp

template <typename... Types>
struct std::formatter<std::variant<Types...>> : cpp::tag_union_formatter { };

template <>
struct std::formatter<std::monostate> : cpp::universal_formatter { };



