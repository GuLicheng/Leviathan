#pragma once

#include "internal.hpp"

namespace nom::bytes
{

inline constexpr auto take_till = []<typename Prediction>(Prediction&& pred) static
{
    return make_parser_binder(TakeTill<false>(), (Prediction&&)pred);
};

inline constexpr auto take_till1 = []<typename Prediction>(Prediction&& pred) static
{
    return make_parser_binder(TakeTill<true>(), (Prediction&&)pred);
};

inline constexpr struct
{
    template <typename Normal, typename ControlChar, typename Escapable>
    static constexpr auto operator()(Normal&& n, ControlChar&& c, Escapable&& e)
    {
        return Escaped<std::decay_t<Normal>, std::decay_t<ControlChar>, std::decay_t<Escapable>>(
            (Normal&&)n, (ControlChar&&)c, (Escapable&&)e
        );
    }
} escaped;

}   // namespace nom::bytes