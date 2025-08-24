/* 
    We only implement a very small subset of nom with nom::complete part.
    The streaming part is not implemented.
*/

#pragma once

#include "error.hpp"

namespace nom
{

template <typename ParseContext>
struct Parser 
{
    // using Output = ...
    // using Error = ... requires ParserError<ParseContext>

    // template <typename Self, typename ParseContext>
    // auto parse(this Self&& self, ParseContext& ctx) const
    // {
    //     return std::invoke(std::forward<Self>(self), ctx);
    // }
};

struct None { };

class Tag
{
    std::string_view value;

public:

    constexpr Tag(std::string_view v) : value(v) { }

    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx) const
    {
        if (ctx.starts_with(value))
        {
            ctx.remove_prefix(value.size());
            return value;
        }
        else
        {
            return IResult<std::string_view>(
                std::unexpect, 
                std::format("Tag parser failed: expected '{}', but got {}.", value, ctx),
                ErrorKind::Tag
            );
        }
    }
};
    
inline constexpr struct 
{
    static constexpr auto operator()(const char* input)
    {
        std::string_view sv(input);
        return Tag(sv);
    }

    static constexpr auto operator()(std::string_view input)
    {
        return Tag(input);
    }
} tag;



}  // namespace nom
