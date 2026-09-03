/*
    https://docs.rs/winnow/latest/winnow/combinator/index.html

    - alt
    - backtrack_err
    - [x] cond
    - cut_err
    - delimited
    - [x] empty
    - [x] eof
    - [x] expression
    - [x] fail
    - [x] fill
    - [x] iterator
    - not
    - opt
    - peek
    - preceded
    - [x] repeat
    - [x] repeat_till
    - [x] separated
    - [x] separated_foldl1
    - [x] separated_foldr1
    - separated_pair
    - terminated
    - [x] todo
    - [x] trace

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

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::backtrack_err_parser<Parser>(std::move(parser));
    }
} backtrack_err;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::cut_err_parser<Parser>(std::move(parser));
    }
} cut_err;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::peek_parser<Parser>(std::move(parser));
    }
} peek;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::opt_parser<Parser>(std::move(parser));
    }
} opt;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::not_parser<Parser>(std::move(parser));
    }
} not_;

inline constexpr struct
{
    template <typename... Parsers>
    static constexpr auto operator()(Parsers... parsers)
    {
        return detail::choice_parser<Parsers...>(std::move(parsers)...);
    }
} alt;

}  // namespace winnow::combinator
