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

    
    TODO:
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
        return [=]<typename Stream>(Stream& stream) 
        {
            static_assert(std::is_same_v<CharT, typename Stream::value_type>, "Stream value type must match literal type");
            return detail::literal_parser<Stream>(str)(stream);
        };
    }
} literal;

inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred pred, size_t min, std::optional<size_t> max = std::nullopt)
    {
        return [=]<typename Stream>(Stream& stream)
        {
            // auto fn = [](auto& pred, const Stream& c) { return !std::invoke(pred, c); };
            return detail::take_while_parser<Stream, Pred>(pred, min, max)(stream);
        };
    }
} take_while;
    
inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred pred, size_t min, std::optional<size_t> max = std::nullopt)
    {
        return [=]<typename Stream>(Stream& stream)
        {
            auto fn = std::not_fn(std::move(pred));
            return detail::take_while_parser<Stream, decltype(fn)>(std::move(fn), min, max)(stream);
        };
    }
} take_till;

inline constexpr struct
{
    static constexpr auto operator()(size_t count)
    {
        return [=]<typename Stream>(Stream& stream)
        {
            return detail::take_parser<Stream>(count)(stream);
        };
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
        return [=]<typename Stream>(Stream& stream) 
        {
            static_assert(std::is_same_v<CharT, typename Stream::value_type>, "Stream value type must match literal type");
            return detail::take_until_parser<Stream>(str, min, max)(stream);
        };
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












}  // namespace winnow::token
