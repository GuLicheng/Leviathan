#pragma once

#include "internal.hpp"

namespace nom::multi
{

inline constexpr struct
{
    template <typename SepParser, typename ItemParser>
    static constexpr auto operator()(SepParser&& sp, ItemParser&& ip)
    {
        return make_parser_binder(SeparatedList<false>(), (SepParser&&)sp, (ItemParser&&)ip);
    }
} separated_list0;

inline constexpr struct
{
    template <typename SepParser, typename ItemParser>
    static constexpr auto operator()(SepParser&& sp, ItemParser&& ip)
    {
        return make_parser_binder(SeparatedList<true>(), (SepParser&&)sp, (ItemParser&&)ip);
    }
} separated_list1;

}  // namespace nom::multi
