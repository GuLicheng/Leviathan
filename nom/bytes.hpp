/*
    https://docs.rs/nom/latest/nom/bytes/index.html

    Implemented:
    - tag
    - take_while

    TODO:
    - escaped
    - escaped_transform
    - is_a
    - is_not
    - tag_no_case
    - take
    - take_till
    - take_till1
    - take_until
    - take_until1
    - take_while1
    - take_while_m_n

*/

#pragma once

#include "internal.hpp"

namespace nom::bytes
{

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* t)
    {
        return operator()(std::basic_string_view<CharT>(t));
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> t)
    {
        return [=]<typename Context>(Context ctx) 
        {
            return detail::tag_parser<Context>(t)(std::move(ctx));
        };
    }
} tag;

inline constexpr auto take_while = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        return detail::loop_parser<Context, Pred>(std::move(pred))(std::move(ctx));
    };
};

inline constexpr auto take_while1 = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        return detail::loop_parser1<Context, Pred>(std::move(pred), error_kind::take_while1)(std::move(ctx));
    };
};

inline constexpr auto take_till = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        auto fn = std::not_fn(std::move(pred));
        return detail::loop_parser<Context, decltype(fn)>(std::move(fn))(std::move(ctx));
    };
};

inline constexpr auto take_till1 = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        auto fn = std::not_fn(std::move(pred));
        return detail::loop_parser1<Context, decltype(fn)>(std::move(fn), error_kind::take_till1)(std::move(ctx));
    };
};

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* partten)
    {
        return operator()(std::basic_string_view<CharT>(partten));
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> partten)
    {
        return [=]<typename Context>(Context ctx) 
        {
            auto searcher = [=](CharT ch) { return partten.contains(ch); };
            return detail::loop_parser1<Context, decltype(searcher)>(std::move(searcher), error_kind::is_a)(std::move(ctx));
        };
    }
} is_a;

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* partten)
    {
        return operator()(std::basic_string_view<CharT>(partten));
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> partten)
    {
        return [=]<typename Context>(Context ctx) 
        {
            auto searcher = [=](CharT ch) { return !partten.contains(ch); };
            return detail::loop_parser1<Context, decltype(searcher)>(std::move(searcher), error_kind::is_not)(std::move(ctx));
        };
    }
} is_not;

inline constexpr auto take = [](size_t count) static
{
    return [=]<typename Context>(Context ctx) 
    {
        return detail::take_parser<Context>(count)(std::move(ctx));
    };
};

inline constexpr auto escaped = []<typename Normal, typename ControlChar, typename Escapable>(Normal n, ControlChar c, Escapable e) static
{
    return [n = std::move(n), c = std::move(c), e = std::move(e)]<typename Context>(Context ctx) 
    {
        return detail::escaped_parser<Context, Normal, ControlChar, Escapable>(std::move(n), std::move(c), std::move(e))(std::move(ctx));
    };
};

}  // namespace nom::bytes
