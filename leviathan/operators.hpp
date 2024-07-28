#pragma once

#include <type_traits>
#include <utility>

namespace leviathan::operators
{

template <typename T>
inline constexpr bool enum_enable_pipe = false;

// template <typename T>
// inline constexpr bool variant_enable_equal = false;

}

template <typename Enum>
    requires (std::is_enum_v<Enum> && leviathan::operators::enum_enable_pipe<Enum>)
constexpr Enum operator|(Enum x, Enum y)
{
    return Enum(std::to_underlying(x) | std::to_underlying(y));
}

// template <typename Variant>
//     requires (leviathan::operators::variant_enable_equal<Variant>)
// constexpr bool operator==(const Variant& x, const Variant& y)
// {
//     return std::visit([]<typename T1, typename T2>(const T1& x1, const T2& 2) {
//         if constexpr (std::is_same_v<T1, T2>)
//         {
//             return x1 == x2;
//         }
//         else
//         {
//             return false;
//         }
//     });
// }

