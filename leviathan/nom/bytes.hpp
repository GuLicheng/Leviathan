/*
    TODO:

    - escaped_transform
    - tag_no_case
    - take_until
    - take_until1
    - take_while_m_n
*/
#pragma once

#include <leviathan/config_parser/context.hpp>
#include "internal.hpp"


namespace nom::bytes
{

inline constexpr auto tag = []<typename StringLike>(StringLike tv)
{
    return detail::tag_fn(tv);
};

inline constexpr auto take_while0 = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        return detail::conditional_loop0<Context, Pred>(std::move(pred))(std::move(ctx));
    };
};

inline constexpr auto take_while1 = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        return detail::conditional_loop1<Context, Pred>(std::move(pred), error_kind::take_while1)(std::move(ctx));
    };
};

inline constexpr auto take_till0 = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        auto fn = std::not_fn(std::move(pred));
        return detail::conditional_loop0<context, decltype(fn)>(std::move(fn))(std::move(ctx));
    };
};

inline constexpr auto take_till1 = []<typename Pred>(Pred pred) static
{
    return [pred = std::move(pred)]<typename Context>(Context ctx) 
    {
        auto fn = std::not_fn(std::move(pred));
        return detail::conditional_loop1<context, decltype(fn)>(std::move(fn), error_kind::take_till1)(std::move(ctx));
    };
};

inline constexpr auto is_a = []<typename StringLike>(StringLike tv) static
{
    return detail::is_a_fn(tv);
};

inline constexpr auto is_not = []<typename StringLike>(StringLike tv) static
{
    return detail::is_not_fn(tv);
};


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