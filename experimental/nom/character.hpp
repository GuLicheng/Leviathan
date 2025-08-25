#pragma once

#include "internal.hpp"

namespace nom::character
{

// inline constexpr auto multispace0 = MultiSpace<false>();
// inline constexpr auto multispace1 = MultiSpace<true>();

inline constexpr auto multispace0 = Conditional2<false, IsMultiSpace, ErrorKind::Ok>();
inline constexpr auto multispace1 = Conditional2<true, IsMultiSpace, ErrorKind::MultiSpace>();

inline constexpr auto digit0 = Dight<false>();
inline constexpr auto digit1 = Dight<true>();

inline constexpr auto one_of = [](std::string_view s) static
{
    return OneOf(s);
};

inline constexpr auto char_ = [](char ch) static
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

inline constexpr auto satisfy = Satisfy();

inline constexpr auto alphanumeric0 = Alphanumeric<false>();
inline constexpr auto alphanumeric1 = Alphanumeric<true>();

inline constexpr auto alpha0 = Alpha<false>();
inline constexpr auto alpha1 = Alpha<true>();

inline constexpr auto space0 = Space<false>();
inline constexpr auto space1 = Space<true>();

inline constexpr auto tab = char_('\t');
inline constexpr auto newline = char_('\n');

}  // namespace nom

