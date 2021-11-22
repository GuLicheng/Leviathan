#pragma once

#include "./stl_ranges.hpp"

namespace leviathan::views
{

    inline constexpr auto take_last_while = []<typename Pred>(Pred pred)
    {
        return reverse
            | take_while(std::move(pred))
            | reverse;
    };

}  // namespace views


