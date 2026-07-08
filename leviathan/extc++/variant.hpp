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

consteval std::meta::info variant_append(std::meta::info type, std::meta::info info)
{
    auto params = std::meta::template_arguments_of(type);
    params.emplace_back(info);
    return std::meta::substitute(std::meta::template_of(type), params);
}
    
template <size_t N> struct undefined;

// Class ^^Undefined
template <std::meta::info Class>
struct variant_builder 
{
    static consteval bool is_defined(size_t index) 
    {
        return is_complete_type(substitute(Class, {std::meta::reflect_constant(index)}));
    }

    static consteval size_t get_last_index() 
    {
        size_t k = 0;
        for (; is_defined(k); ++k);
        return k;
    }

    static consteval void put(std::meta::info info)
    {
        auto index = get_last_index();
        define_aggregate(
            substitute(Class, { std::meta::reflect_constant(index) }),
            { std::meta::data_member_spec(info, { .name = "value" }) }
        );
    }

    template <size_t Index = get_last_index() - 1>
    static consteval std::meta::info current()
    {
        return type_of(^^undefined<Index>::value);
    }

    template <size_t Index = get_last_index() - 1>
    using get_t = decltype(undefined<Index>::value);

    template <typename T>
    static consteval void declare()
    {
        put(variant_append(current<>(), ^^T));
    }
};

} // namespace detail

using variant_builder = detail::variant_builder<^^detail::undefined>;

consteval { variant_builder::put(^^std::variant<std::monostate>); }

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



