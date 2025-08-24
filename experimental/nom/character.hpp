#pragma once

#include "internal.hpp"

namespace nom::character
{

inline constexpr auto multispace0 = MultiSpace<false>();
inline constexpr auto multispace1 = MultiSpace<true>();

inline constexpr auto digit0 = Dight<false>();
inline constexpr auto digit1 = Dight<true>();

inline constexpr auto one_of = [](std::string_view s) static
{
    return OneOf(s);
};

}  // namespace nom

