#pragma once

#include <type_traits>
#include <utility>

namespace cpp::operators
{

template <typename T>
inline constexpr bool enum_enable_bitop = false && std::is_enum_v<T>;

}  // namespace cpp::operators

template <typename Enum>
    requires cpp::operators::enum_enable_bitop<Enum>
constexpr Enum operator|(Enum x, Enum y)
{
    return Enum(std::to_underlying(x) | std::to_underlying(y));
}

template <typename Enum>
    requires cpp::operators::enum_enable_bitop<Enum>
constexpr Enum& operator|=(Enum& x, Enum y)
{
    x = Enum(std::to_underlying(x) | std::to_underlying(y));
    return x;
}

template <typename Enum>
    requires cpp::operators::enum_enable_bitop<Enum>
constexpr Enum operator&(Enum x, Enum y)
{
    return Enum(std::to_underlying(x) & std::to_underlying(y));
}

template <typename Enum>
    requires cpp::operators::enum_enable_bitop<Enum>
constexpr Enum& operator&=(Enum& x, Enum y)
{
    x = Enum(std::to_underlying(x) & std::to_underlying(y));
    return x;
}

