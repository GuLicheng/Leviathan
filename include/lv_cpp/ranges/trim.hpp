#pragma once


#include "./drop_last_while.hpp"

#include <cctype>


namespace leviathan::views
{

inline constexpr auto trim_front = []<typename Pred>(Pred pred)
{
    return drop_while(std::move(pred));
};

inline constexpr auto trim_back = []<typename Pred>(Pred pred)
{
    return drop_last_while(std::move(pred));
};



inline constexpr auto trim = []<typename Pred>(Pred pred)
{
    return trim_front(pred) | trim_back(std::move(pred));
};



} // namespace leviathan
