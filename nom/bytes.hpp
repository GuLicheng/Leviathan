/*
    https://docs.rs/nom/latest/nom/bytes/index.html

    Implemented:


    TODO:
    - escaped
    - escaped_transform
    - is_a
    - is_not
    - tag
    - tag_no_case
    - take
    - take_till
    - take_till1
    - take_until
    - take_until1
    - take_while
    - take_while1
    - take_while_m_n

*/

#pragma once

#include "internal.hpp"

namespace nom::bytes
{

inline constexpr auto tag = []<typename StringLike>(StringLike tv)
{
    return detail::tag_fn(tv);
};

inline constexpr auto take_while = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        return detail::conditional_loop0<Context, Pred>(std::move(pred))(std::move(ctx));
    };
};

}  // namespace nom::bytes
