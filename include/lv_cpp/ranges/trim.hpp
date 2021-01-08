#ifndef __TRIM_HPP__
#define __TRIM_HPP__

#include <ranges>
#include <cctype>

namespace leviathan::ranges
{


namespace views
{


namespace detail
{
inline constexpr auto trim_front = ::std::ranges::views::drop_while(::isspace);

inline constexpr auto trim_back = ::std::ranges::views::reverse
                                | ::std::ranges::views::drop_while(::isspace)
                                | ::std::ranges::views::reverse;

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


}  // namespace ranges

namespace leviathan
{
    namespace views = ::leviathan::ranges::views;
}

#endif