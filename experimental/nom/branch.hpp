#pragma once

#include "internal.hpp"

// https://docs.rs/nom/latest/nom/branch/index.html
namespace nom::branch
{

inline constexpr struct 
{
    template <typename... Fns>
    constexpr auto operator()(Fns&&... fns) const
    {
        return make_parser_binder(Choice(), (Fns&&)fns...);
    }
} alt;

// TODO: implement 'permutation'

}  // namespace nom::branch