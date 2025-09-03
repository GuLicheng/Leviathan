#pragma once

#include "internal.hpp"

namespace nom::multi
{

inline constexpr auto separated_list0 = 
    []<typename SepParser, typename ItemParser>(SepParser sp, ItemParser ip) static
{
    return [sp = std::move(sp), ip = std::move(ip)]<typename Context>(Context ctx) 
    {
        return detail::separated_list_parser<Context, SepParser, ItemParser, false>(std::move(sp), std::move(ip))(ctx);
    };
};

inline constexpr auto separated_list1 = 
    []<typename SepParser, typename ItemParser>(SepParser sp, ItemParser ip) static
{
    return [sp = std::move(sp), ip = std::move(ip)]<typename Context>(Context ctx) 
    {
        return detail::separated_list_parser<Context, SepParser, ItemParser, true>(std::move(sp), std::move(ip))(ctx);
    };
};

inline constexpr auto many0 = []<typename ItemParser>(ItemParser ip) static
{
    return [ip = std::move(ip)]<typename Context>(Context ctx) 
    {
        return detail::many_parser<Context, ItemParser, false>(std::move(ip))(std::move(ctx));
    };
};

inline constexpr auto many1 = []<typename ItemParser>(ItemParser ip) static
{
    return [ip = std::move(ip)]<typename Context>(Context ctx) 
    {
        return detail::many_parser<Context, ItemParser, true>(std::move(ip))(std::move(ctx));
    };
};

}  // namespace nom::multi