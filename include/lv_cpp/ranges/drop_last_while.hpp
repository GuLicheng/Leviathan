#pragma once

#include <ranges>

namespace leviathan::views
{

inline constexpr auto drop_last_while = []<typename Pred>(Pred pred)
{
    return ::std::views::reverse 
         | ::std::views::drop_while(std::move(pred)) // move is necessary for ::isspace for other functions
         | ::std::views::reverse;
}; 
 
} //  namespace leviathan

