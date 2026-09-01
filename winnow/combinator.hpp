/*
    https://docs.rs/winnow/latest/winnow/combinator/index.html

    Implemented

    - preceded
    - terminated
    - delimited
    - separated_pair

*/

#pragma once

#include "internal.hpp"

namespace winnow::combinator
{

inline constexpr struct
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1 f1, F2 f2)
    {
        return detail::preceded_parser<F1, F2>(std::move(f1), std::move(f2));
    }
} preceded;

inline constexpr struct
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1 f1, F2 f2)
    {
        return detail::terminated_parser<F1, F2>(std::move(f1), std::move(f2));
    }
} terminated;

inline constexpr struct
{
    template <typename F1, typename F2, typename F3>
    static constexpr auto operator()(F1 f1, F2 f2, F3 f3)
    {
        auto parser1 = preceded(std::move(f1), std::move(f2));
        return terminated(std::move(parser1), std::move(f3));
    }
} delimited;

inline constexpr struct
{
    template <typename F1, typename F2, typename F3>
    static constexpr auto operator()(F1 f1, F2 f2, F3 f3)
    {
        return detail::separated_pair_parser<F1, F2, F3>(std::move(f1), std::move(f2), std::move(f3));
    }
} separated_pair;


}  // namespace winnow::combinator
