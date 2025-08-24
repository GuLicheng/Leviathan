#pragma once

#include "internal.hpp"

namespace nom::branch
{

inline constexpr struct 
{
    template <typename... Fns>
    constexpr auto operator()(Fns&&... fns) const
    {
        return Choice<std::decay_t<Fns>...>((Fns&&)fns...);
    }
} alt;


}  // namespace nom::branch