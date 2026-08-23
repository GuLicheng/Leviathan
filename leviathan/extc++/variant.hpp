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
            return std::format_to(ctx.out(), "{}({})", display_string_of(^^Variant), value);
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
class variant_builder 
{
    static consteval bool is_defined(size_t index) 
    {
        return is_complete_type(substitute(Class, {std::meta::reflect_constant(index)}));
    }

    static consteval size_t size() 
    {
        size_t k = 0;
        for (; is_defined(k); ++k);
        return k;
    }

public:

    static consteval void put(std::meta::info info)
    {
        auto index = size();
        define_aggregate(
            substitute(Class, { std::meta::reflect_constant(index) }),
            { std::meta::data_member_spec(info, { .name = "value" }) }
        );
    }

    // Template is necessary since the consteval function
    // maybe evaluated only once, so we must keep sth changing 
    // to make it evaluated again.
    template <size_t Index = size() - 1>
    static consteval std::meta::info current()
    {
        return type_of(^^undefined<Index>::value);
    }

    template <typename T>
    static consteval void declare()
    {
        put(variant_append(current(), ^^T));
    }
};

} // namespace detail

using variant_builder = detail::variant_builder<^^detail::undefined>;

consteval { variant_builder::put(^^std::variant<std::monostate>); }

/*
consteval {
    variant_builder::declare<double>();
    variant_builder::declare<bool>();
}
*/

}  // namespace cpp

template <typename... Types>
struct std::formatter<std::variant<Types...>> : cpp::tag_union_formatter { };

template <>
struct std::formatter<std::monostate> : cpp::universal_formatter { };



