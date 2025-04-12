#pragma once

#include "common.hpp"

#include <ranges>

namespace cpp::config
{

template <typename... Args>
std::string format(std::string_view fmt, Args&&... args)
{
    return std::vformat(fmt, std::make_format_args(args...));
}

template <typename Formatter, typename Sequence>
std::string format_sequence(
    Formatter formatter, 
    const Sequence& sequence, 
    std::string_view separator = ",", 
    std::string_view fmt = "[{}]")
{
    auto context = sequence 
                 | std::views::transform(formatter)
                 | std::views::join_with(separator) 
                 | std::ranges::to<std::string>();
    return format(fmt, context);
}

template <typename Formatter, typename Map>
std::string format_map(
    Formatter formatter, 
    const Map& map, 
    std::string_view kvfmt = "{}:{}", 
    std::string_view separator = ",", 
    std::string_view fmt = "{{{}}}")
{
    auto kv2string = [=](auto& kv) {
        return format(kvfmt,
            Formatter::operator()(kv.first), 
            Formatter::operator()(kv.second));
    };

    auto context = map 
                 | std::views::transform(kv2string) 
                 | std::views::join_with(separator) 
                 | std::ranges::to<std::string>();
    return format(fmt, context);
}


} // namespace cpp::config
