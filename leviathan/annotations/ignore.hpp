#pragma once

#include <leviathan/annotations/common.hpp>

namespace cpp::refl
{

struct ignore_annotation : annotation { };

inline constexpr auto ignore = ignore_annotation{};

}
