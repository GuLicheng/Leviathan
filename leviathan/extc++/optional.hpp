#pragma once

#include <leviathan/extc++/format.hpp>

namespace cpp
{


}


template <typename T>
struct std::formatter<std::optional<T>>
{
    template <typename ParseContext>
    static constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    { 
        return ctx.begin();
    }


    template <typename FormatContext>
    auto format(const std::optional<T>& opt, FormatContext& ctx)
    {
        if (opt)
        {
            return std::formatter<T>::format(*opt, ctx);
        }
        else
        {
            return format_to(ctx.out(), "nullopt");
        }
    }
};
