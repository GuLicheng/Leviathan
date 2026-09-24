#pragma once

#include <utility>

namespace cpp
{

struct none_t { explicit none_t() = default; };

inline constexpr none_t none{};

}
