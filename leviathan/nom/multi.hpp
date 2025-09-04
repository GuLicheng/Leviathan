/*
    TODO:

    - fold
    - fold_many_m_n
    - length_count
    - length_data
    - length_value
    - many
    - many0_count
    - many1_count
    - many_m_n
    - many_till
*/

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

inline constexpr auto count = []<typename ItemParser>(ItemParser ip, size_t cnt) static
{
    return [ip = std::move(ip), cnt]<typename Context>(Context ctx) 
    {
        return detail::count_parser<Context, ItemParser>(cnt, std::move(ip))(std::move(ctx));
    };
};

inline constexpr auto fill = 
    []<typename ItemParser, typename OutputIt, typename Sentinel>(ItemParser ip, OutputIt out, Sentinel sent) static
{
    return [ip = std::move(ip), out = std::move(out), sent = std::move(sent)]<typename Context>(Context ctx) 
    {
        return detail::fill_parser<Context, ItemParser, OutputIt, Sentinel>(std::move(ip), std::move(out), std::move(sent))(std::move(ctx));
    };
};

inline constexpr auto fold_many0 = 
    []<typename ItemParser, typename Init, typename Fn>(ItemParser ip, Init init, Fn fn) static
{
    return [ip = std::move(ip), init = std::move(init), fn = std::move(fn)]<typename Context>(Context ctx) 
    {
        return detail::many_fold_parser<Context, ItemParser, Init, Fn, false>(std::move(ip), std::move(fn), std::move(init))(std::move(ctx));
    };
};

inline constexpr auto fold_many1 = 
    []<typename ItemParser, typename Init, typename Fn>(ItemParser ip, Init init, Fn fn) static
{
    return [ip = std::move(ip), init = std::move(init), fn = std::move(fn)]<typename Context>(Context ctx) 
    {
        return detail::many_fold_parser<Context, ItemParser, Init, Fn, true>(std::move(ip), std::move(fn), std::move(init))(std::move(ctx));
    };
};

}  // namespace nom::multi