/*
    https://docs.rs/winnow/latest/winnow/token/index.html

    Implemented:
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
    
    TODO:

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
        return detail::take_while_parser<Pred>(std::move(pred), min, max);
    }
} take_while;
    
inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred pred, size_t min = 0, std::optional<size_t> max = std::nullopt)
    {
        auto fn = std::not_fn(std::move(pred));
        return detail::take_while_parser<decltype(fn)>(std::move(fn), min, max);
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
        return detail::take_until_parser<CharT>(str, min, max);
    }
} take_until;

inline constexpr struct
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using literal_type = std::basic_string_view<typename Stream::value_type>;
        using error_type = typename Stream::error_type;
        using result_type = modal_result<literal_type, error_type>;

        auto left = stream.to_string_view();
        stream.advance(stream.size());
        return result_type::make_ok(left);
    }
} rest;

inline constexpr struct 
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<size_t, error_type>;
        return result_type::make_ok(stream.size());
    }
} rest_len;

inline constexpr struct
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using literal_type = std::basic_string_view<typename Stream::value_type>;
        using error_type = typename Stream::error_type;
        using result_type = modal_result<literal_type, error_type>;

        if (stream.size() == 0)
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }
        else
        {
            auto [left, right] = stream.split_at(1);
            stream = right;
            return result_type::make_ok(std::move(left));
        }
    }
} any;

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
        return [tokens=std::move(tokens)]<typename Stream>(Stream& stream) 
        {
            // Different from Rust, we return a string_view instead of a char.
            using error_type = typename Stream::error_type;
            using result_type = modal_result<std::basic_string_view<typename Stream::value_type>, error_type>;

            if (stream.size() == 0 || std::ranges::contains(tokens, stream[0]))
            {
                return result_type::make_err(
                    err_mode<error_type>::make_backtrack(
                        error_traits<error_type>::from_input(stream)
                    )
                );
            }
            auto [left, right] = stream.split_at(1);
            stream = right;
            return result_type::make_ok(std::move(left));
        };
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
        return [tokens=std::move(tokens)]<typename Stream>(Stream& stream) 
        {
            // Different from Rust, we return a string_view instead of a char.
            using error_type = typename Stream::error_type;
            using result_type = modal_result<std::basic_string_view<typename Stream::value_type>, error_type>;

            if (stream.size() == 0 || !std::ranges::contains(tokens, stream[0]))
            {
                return result_type::make_err(
                    err_mode<error_type>::make_backtrack(
                        error_traits<error_type>::from_input(stream)
                    )
                );
            }
            auto [left, right] = stream.split_at(1);
            stream = right;
            return result_type::make_ok(std::move(left));
        };
    }
} one_of;

}  // namespace winnow::token
