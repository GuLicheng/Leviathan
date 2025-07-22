#pragma once

#include <format>
#include <variant>

template <typename... Ts, typename CharT>
struct std::formatter<std::variant<Ts...>, CharT>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(const std::variant<Ts...>& v, FormatContext& ctx) const
    {
        return std::visit(
            [&](const auto& value) { 
                return std::format_to(ctx.out(), "{}", value);
            }, v
        );
    }
};

