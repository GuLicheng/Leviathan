#pragma once

#include <format>
#include <ranges>
#include <vector>

namespace leviathan::collections
{
    
struct node_drawer
{
    template <typename Self>
    std::string draw(this const Self& self)
    {
        const auto fn1 = [](auto it) static 
        {
            return std::make_pair(it.level(), std::format(" -> {}", *it)); 
        };

        const auto value_and_level = std::views::iota(self.begin(), self.end()) 
                                   | std::views::transform(fn1) 
                                   | std::ranges::to<std::vector>();

        const auto draw_current_level = [&](int level) 
        {
            auto fn2 = [&](auto vl) { return vl.first > level ? vl.second : std::string(vl.second.size(), ' '); };
            auto context = value_and_level | std::views::transform(fn2) | std::views::join | std::ranges::to<std::string>();
            return std::format("head({}){}", level, context);
        };

        return std::views::iota(0, self.level()) 
             | std::views::reverse
             | std::views::transform(draw_current_level)
             | std::views::join_with('\n')
             | std::ranges::to<std::string>(); 
    }
};

} // namespace leviathan::collections

