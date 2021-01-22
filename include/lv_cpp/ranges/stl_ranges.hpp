#pragma once

#include <ranges>

namespace leviathan::views
{
template <typename T> inline constexpr auto& empty = ::std::views::empty<T>;
inline constexpr auto& single = ::std::views::single;
inline constexpr auto& iota = ::std::views::iota;
inline constexpr auto& all = ::std::views::all;
inline constexpr auto& filter = ::std::views::filter;
inline constexpr auto& transform = ::std::views::transform;
inline constexpr auto& take = ::std::views::take;
inline constexpr auto& drop = ::std::views::drop;
inline constexpr auto& take_while = ::std::views::take_while;
inline constexpr auto& drop_while = ::std::views::drop_while;
inline constexpr auto& join = ::std::views::join;
inline constexpr auto& split = ::std::views::split;
inline constexpr auto& counted = ::std::views::counted;
inline constexpr auto& common = ::std::views::common;
inline constexpr auto& reverse = ::std::views::reverse;
inline constexpr auto& keys = ::std::views::keys;
inline constexpr auto& values = ::std::views::values;
template <size_t N> inline constexpr auto& elements = ::std::views::elements<N>;

}
