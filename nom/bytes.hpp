/*
    https://docs.rs/nom/latest/nom/bytes/index.html

    Implemented:
    - tag
    - take_while

    TODO:
    - escaped
    - escaped_transform
    - is_a
    - is_not
    - tag_no_case
    - take
    - take_till
    - take_till1
    - take_until
    - take_until1
    - take_while1
    - take_while_m_n

*/

#pragma once

#include "internal.hpp"

namespace nom::bytes
{

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* t)
    {
        return operator()(std::basic_string_view<CharT>(t));
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> t)
    {
        return [=]<typename Context>(Context ctx) 
        {
            return detail::tag_parser<Context>(t)(std::move(ctx));
        };
    }
} tag;

inline constexpr auto take_while = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        return detail::loop_parser<Context, Pred, false>(std::move(pred))(std::move(ctx));
    };
};

inline constexpr auto take_while1 = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        return detail::loop_parser<Context, Pred, true>(std::move(pred))(std::move(ctx));
    };
};

}  // namespace nom::bytes
