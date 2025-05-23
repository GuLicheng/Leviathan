#pragma once

#include <type_traits>
#include <utility>

namespace cpp::operators
{

template <typename T>
inline constexpr bool enum_enable_pipe = false;

template <typename Enum>
concept pipeable_enum = std::is_enum_v<Enum> && enum_enable_pipe<Enum>;

}  // namespace cpp::operators

template <cpp::operators::pipeable_enum Enum>
constexpr Enum operator|(Enum x, Enum y)
{
    return Enum(std::to_underlying(x) | std::to_underlying(y));
}


