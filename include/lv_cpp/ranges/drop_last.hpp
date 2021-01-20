#pragma once

#include <ranges>

namespace leviathan::views
{
    
inline constexpr auto drop_last = []<typename _Tp>(_Tp&& __n)
{
    return ::std::views::reverse 
         | ::std::views::drop(::std::forward<_Tp>(__n))
         | ::std::views::reverse;  
};

} // namespace leviathan

