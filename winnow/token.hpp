/*
    https://docs.rs/winnow/latest/winnow/token/index.html

    - literal
    - take_while
    - take_till
    - take
    - take_until
    - rest
    - rest_len
    - any
    - none_of
    - one_of

*/

#pragma once

#include "internal.hpp"

namespace winnow::token
{

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* str)
    {
        return operator()(std::basic_string_view<CharT>(str));
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> str)
    {
        return detail::literal_parser<CharT>(str);
    }
} literal;

inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred pred, size_t min = 0, std::optional<size_t> max = std::nullopt)
    {
        return detail::take_while_parser<Pred>(std::move(pred), occurrences<size_t>{min, max});
    }
} take_while;
    
inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred pred, size_t min = 0, std::optional<size_t> max = std::nullopt)
    {
        auto fn = std::not_fn(std::move(pred));
        return detail::take_while_parser<decltype(fn)>(std::move(fn), occurrences<size_t>{min, max});
    }
} take_till;

inline constexpr struct
{
    static constexpr auto operator()(size_t count)
    {
        return detail::take_parser(count);
    }
} take;

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* str, size_t min = 0, std::optional<size_t> max = std::nullopt)
    {
        return operator()(std::basic_string_view<CharT>(str), min, max);
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> str, size_t min = 0, std::optional<size_t> max = std::nullopt)
    {
        return detail::take_until_parser<CharT>(str, occurrences<size_t>{min, max});
    }
} take_until;

inline constexpr detail::rest_parser rest;

inline constexpr detail::rest_len_parser rest_len;

inline constexpr auto any = detail::check_next_character_parser<detail::always_false>(detail::always_false());

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* str)
    {
        return operator()(std::basic_string_view<CharT>(str));
    }

    template <typename Tokens>
    static constexpr auto operator()(Tokens tokens)
    {
        auto contains = [tokens = std::move(tokens)](auto c) { return std::ranges::contains(tokens, c); };
        return detail::check_next_character_parser<decltype(contains)>(std::move(contains));
    }
} none_of;

inline constexpr struct 
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* str)
    {
        return operator()(std::basic_string_view<CharT>(str));
    }

    template <typename Tokens>
    static constexpr auto operator()(Tokens tokens)
    {
        auto contains = [tokens = std::move(tokens)](auto c) { return !std::ranges::contains(tokens, c); };
        return detail::check_next_character_parser<decltype(contains)>(std::move(contains));
    }
} one_of;

}  // namespace winnow::token
