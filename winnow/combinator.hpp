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
        return [f1 = std::move(f1), f2 = std::move(f2)]<typename Stream>(Stream& stream)
        {
            return detail::preceded_parser<Stream, F1, F2>(std::move(f1), std::move(f2))(stream);
        };
    }
} preceded;

inline constexpr struct
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1 f1, F2 f2)
    {
        return [f1 = std::move(f1), f2 = std::move(f2)]<typename Stream>(Stream& stream)
        {
            return detail::terminated_parser<Stream, F1, F2>(std::move(f1), std::move(f2))(stream);
        };
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
        return [f1 = std::move(f1), f2 = std::move(f2), f3 = std::move(f3)]<typename Stream>(Stream& stream)
        {
            return detail::separated_pair_parser<Stream, F1, F2, F3>(std::move(f1), std::move(f2), std::move(f3))(stream);
        };
    }
} separated_pair;

template <typename F>
struct map_view
{
    F func;

    constexpr map_view(F f) : func(std::move(f)) { }

    template <typename M>
    constexpr friend auto operator|(M m, map_view<F> mv)
    {
        return [f = std::move(mv.func), m = std::move(m)]<typename Stream>(Stream& stream)
        {
            return detail::map_parser<Stream, M, F>(std::move(m), std::move(f))(stream);
        };
    }
};

inline constexpr struct
{
    template <typename F, typename M>
    static constexpr auto operator()(F f, M m)
    {
        return [f = std::move(f), m = std::move(m)]<typename Stream>(Stream& stream)
        {
            return detail::map_parser<Stream, F, M>(std::move(f), std::move(m))(stream);
        };
    }

    template <typename F>
    static constexpr auto operator()(F f)
    {
        return map_view<F>{ std::move(f) };
    }

} map;

inline constexpr struct 
{
    template <typename F, typename M>
    static constexpr auto operator()(F f, M m)
    {
        return [f = std::move(f), m = std::move(m)]<typename Stream>(Stream& stream)
        {
            return detail::verify_parser<Stream, F, M>(std::move(f), std::move(m))(stream);
        };
    }
} verify;

}  // namespace winnow::combinator
