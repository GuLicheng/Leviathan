#ifndef __DROP_LAST_HPP__
#define __DROP_LAST_HPP__

#include <ranges>

namespace leviathan::views
{

inline constexpr auto drop_last_while = []<typename Pred>(Pred pred)
{
    return ::std::views::reverse 
         | ::std::views::drop_while(pred) 
         | ::std::views::reverse;
}; 
 
} //  namespace leviathan


#endif