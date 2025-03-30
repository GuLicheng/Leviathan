#pragma once

#include <format>
#include <ranges>
#include <vector>

namespace leviathan::collections
{
    
struct node_drawer
{
private:

    constexpr static auto width = 5;

public:

    template <typename Self>
    std::string draw(this const Self& self)
    {
        const auto value_and_level = std::views::zip_transform(
            [](auto it, auto value) { return std::make_pair(it.level(), std::move(value)); },
            std::views::iota(self.begin(), self.end()),
            self | std::views::transform([](auto x) { return std::format(" -> {:{}}", x, width); })
        ) | std::ranges::to<std::vector>();
        
        const auto blank = " -> " + std::string(width, ' ');

        auto draw_current_level = [&](int level) 
        {
            auto fn = [&](auto vl) { return vl.first > level ? vl.second : blank; };
            auto context = value_and_level | std::views::transform(fn) | std::views::join | std::ranges::to<std::string>();
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

