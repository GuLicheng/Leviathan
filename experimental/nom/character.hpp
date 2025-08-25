#pragma once

#include "internal.hpp"

namespace nom::character
{

inline constexpr auto multispace0 = MultiSpace<false>();
inline constexpr auto multispace1 = MultiSpace<true>();

inline constexpr auto digit0 = Dight<false>();
inline constexpr auto digit1 = Dight<true>();

inline constexpr auto one_of = [](std::string_view s) static
{
    return OneOf(s);
};

inline constexpr auto char_ = [](char ch) 
{
    return [ch]<typename ParseContext>(ParseContext& ctx) 
    {
        if (ctx.empty() || ctx[0] != ch)
        {
            return IResult<char>(
                std::unexpect, 
                std::format("Expected character: '{}'", ch),
                ErrorKind::Char
            );
        }

        ctx.remove_prefix(1);
        return IResult<char>(std::in_place, ch);
    };
}; 

inline constexpr auto alphanumeric0 = Alphanumeric<false>();
inline constexpr auto alphanumeric1 = Alphanumeric<true>();

}  // namespace nom

