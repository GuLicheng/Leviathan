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

