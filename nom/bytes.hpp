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

}  // namespace nom::bytes
