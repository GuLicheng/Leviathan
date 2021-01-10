#ifndef __TAKE_LAST_WHILE_HPP__
#define __TALE_LAST_WHILE_HPP__

#include <ranges>

namespace leviathan::views
{

inline constexpr auto take_last_while = []<typename Pred>(Pred pred)
{
    return ::std::views::reverse
         | ::std::views::take_while(pred)
         | ::std::views::reverse;
};

}  // namespace views


#endif