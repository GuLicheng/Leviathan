#pragma once

#include "internal.hpp"
#include <leviathan/type_caster.hpp>

namespace nom::number
{

inline constexpr struct
{
    static constexpr bool valid(char ch)
    {
        return std::isdigit(ch) 
             || ch == '.' 
             || ch == '-' 
             || ch == '+' 
             || ch == 'e' 
             || ch == 'E';
    }

    template <typename ParseContext>
    static constexpr auto operator()(ParseContext& ctx)
    {
        auto first = ctx.begin(), last = ctx.end();

        for (; first != last && valid(*first); ++first);

        std::string_view result = { ctx.begin(), first };

        using NumberCaste = cpp::type_caster<double, std::string_view, cpp::error_policy::optional>;

        auto num = NumberCaste::operator()(result);

        if (!num)
        {
            return IResult<std::string_view>(
                std::unexpect, 
                "Failed to parse float number",
                ErrorKind::Float
            );
        }

        ctx.remove_prefix(result.size());
        return IResult<double>(std::in_place, *num);
    }
} double_;

}  // namespace nom::number
