#pragma once

#include "internal.hpp"

namespace nom::bytes
{

inline constexpr struct
{
    template <typename Prediction>
    static constexpr auto operator()(Prediction&& pred)
    {
        return make_parser_binder(TakeTill<false>(), (Prediction&&)pred);
    }
} take_till;

inline constexpr struct
{
    template <typename Prediction>
    static constexpr auto operator()(Prediction&& pred)
    {
        return make_parser_binder(TakeTill<true>(), (Prediction&&)pred);
    }
} take_till1;

inline constexpr struct
{
    template <typename Normal, typename ControlChar, typename Escapable>
    static constexpr auto operator()(Normal&& n, ControlChar&& c, Escapable&& e)
    {
        return make_parser_binder(Escaped(), (Normal&&)n, (ControlChar&&)c, (Escapable&&)e);
    }
} escaped;

}   // namespace nom::bytes