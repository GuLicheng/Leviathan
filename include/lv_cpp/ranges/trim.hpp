#ifndef __TRIM_HPP__
#define __TRIM_HPP__

// #include <ranges>
#include <lv_cpp/ranges/drop_last_while.hpp>

#include <ranges>
#include <cctype>


namespace leviathan
{

namespace views
{


namespace detail
{
inline constexpr auto trim_front = ::std::ranges::views::drop_while(::isspace);

inline constexpr auto trim_back = ::leviathan::views::drop_last_while(::isspace);

} // namespace detail

inline constexpr auto trim = []<typename StringType>(StringType&& str)
{
    return ::std::forward<StringType>(str) | detail::trim_front | detail::trim_back;
};

inline constexpr auto trim_str = []<typename StringType>(StringType&& str) -> std::remove_cvref_t<StringType>
{
    auto new_str = trim(::std::forward<StringType>(str));
    return {new_str.begin(), new_str.end()};
};

} // namespace views

} // namespace leviathan

#endif