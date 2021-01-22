#pragma once

#include "./stl_ranges.hpp"
#include "./zip.hpp"

namespace leviathan::views
{

inline constexpr auto enumerate = []<typename... Ranges>(Ranges&&... rgs)
{
    auto iota_view = iota(0);
    return zip(::std::move(iota_view), ::std::forward<Ranges>(rgs)...);
};


}  // namespace views
