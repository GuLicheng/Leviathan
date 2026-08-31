#pragma once

#include "token.hpp"

namespace winnow::ascii
{

inline constexpr auto alpha0 = token::take_while(::isalpha, 0);
inline constexpr auto alpha1 = token::take_while(::isalpha, 1);

} // namespace winnow::ascii
