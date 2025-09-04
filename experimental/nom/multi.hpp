#pragma once

#include "internal.hpp"

namespace nom::multi
{

// inline constexpr auto separated_list0 = 
//     []<typename SepParser, typename ItemParser>(SepParser&& sp, ItemParser&& ip) static
// {
//     return make_parser_binder(SeparatedList<false>(), (SepParser&&)sp, (ItemParser&&)ip);
// };

// inline constexpr auto separated_list1 = 
//     []<typename SepParser, typename ItemParser>(SepParser&& sp, ItemParser&& ip) static
// {
//     return make_parser_binder(SeparatedList<true>(), (SepParser&&)sp, (ItemParser&&)ip);
// };

// inline constexpr auto many0 = 
//     []<typename ItemParser>(ItemParser&& ip) static
// {
//     return make_parser_binder(Many0(), (ItemParser&&)ip);
// };

inline constexpr auto many1 = 
    []<typename ItemParser>(ItemParser&& ip) static
{
    return make_parser_binder(Many1(), (ItemParser&&)ip);
};

// inline constexpr auto count = 
//     []<typename ItemParser>(ItemParser&& ip, size_t cnt) static
// {
//     return make_parser_binder(Count(), (ItemParser&&)ip, cnt);
// };

// inline constexpr auto fill = 
//     []<typename ItemParser, typename OutputIt, typename Sentinel>(ItemParser&& ip, OutputIt out, Sentinel sent) static
// {
//     return make_parser_binder(Fill(), (ItemParser&&)ip, std::move(out), std::move(sent));
// };

inline constexpr auto fold_many0 = 
    []<typename ItemParser, typename Init, typename Fn>(ItemParser&& ip, Init init, Fn fn) static
{
    return make_parser_binder(ManyFolder(), many0((ItemParser&&)ip), std::move(init), std::move(fn));
};

inline constexpr auto fold_many1 = 
    []<typename ItemParser, typename Init, typename Fn>(ItemParser&& ip, Init init, Fn fn) static
{
    return make_parser_binder(ManyFolder(), many1((ItemParser&&)ip), std::move(init), std::move(fn));
};

}  // namespace nom::multi
