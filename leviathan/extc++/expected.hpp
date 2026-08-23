#pragma once

#include <expected>
#include <format>

namespace cpp
{

struct universal_expected_formatter
{
    template <typename ParseContext>
    static constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    { 
        return ctx.begin();
    }

    template <typename Expected, typename FmtContext>
    static typename FmtContext::iterator format(const Expected& ex, FmtContext& ctx)
    {
        return !!ex 
             ? std::format_to(ctx.out(), "Ok({})", ex.value())
             : std::format_to(ctx.out(), "Err({})", ex.error());
    }
};


}  // namespace cpp

template <typename T, typename E>
struct std::formatter<std::expected<T, E>> : cpp::universal_expected_formatter { };


