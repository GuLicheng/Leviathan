#pragma once

#include <ranges>

namespace leviathan::views
{
    
inline constexpr auto take_last = []<typename T>(T&& __n)
{
    return ::std::views::reverse
         | ::std::views::take(::std::forward<T>(__n))
         | ::std::views::reverse;
};

} // namespace leviathan::views
