#pragma once

#include <expected>
#include <format>

template <typename T, typename E>
struct std::formatter<std::expected<T, E>>
{
    template <typename ParseContext>
    static constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    { 
        return ctx.begin();
    }

    template <typename FmtContext>
    static typename FmtContext::iterator format(const std::expected<T, E>& ex, FmtContext& ctx)
    {
        return ex.has_value() 
             ? std::format_to(ctx.out(), "Ok({})", ex.value())
             : std::format_to(ctx.out(), "Err({})", ex.error());
    }
};