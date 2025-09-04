/*
    Combinators to be implemented:

    x - complete
    x - consumed
    x - cut
    x - flat_map
    x - into
    x - iterator
    x - map_opt
    x - map_res     // unnecessary, use map instead
    x - map_parser  // unnecessary, use map instead
*/

#pragma once

#include <leviathan/extc++/concepts.hpp>
#include "error.hpp"

namespace nom::combinator
{
    
inline constexpr auto value = []<typename T, typename Parser>(T v, Parser p) static
{
    return [v = std::move(v), p = std::move(p)]<typename Context>(Context ctx) 
    {
        return detail::value_parser<Context, T, Parser>(std::move(v), std::move(p))(std::move(ctx));
    };
};

inline constexpr auto eof = []<typename Context>(Context ctx)
{
    return detail::eof_parser<Context>()(std::move(ctx));
};

template <typename T>
inline constexpr auto fail = detail::fail_fn<T>();

inline constexpr auto map = []<typename F, typename M>(F f, M m) static
{
    return [f = std::move(f), m = std::move(m)]<typename Context>(Context ctx)
    {
        return detail::map_parser<Context, F, M>(std::move(f), std::move(m))(std::move(ctx));
    };
};

inline constexpr auto map_parser = []<typename F, typename M>(F f, M m) static
{
    return [f = std::move(f), m = std::move(m)]<typename Context>(Context ctx)
    {
        return detail::map_parser_parser<Context, F, M>(std::move(f), std::move(m))(std::move(ctx));
    };
};

inline constexpr auto opt = []<typename F>(F f) static
{
    return [f = std::move(f)]<typename Context>(Context ctx)
    {
        return detail::opt_parser<Context, F>(std::move(f))(std::move(ctx));
    };
};

inline constexpr auto peek = []<typename F>(F f) static
{
    return [f = std::move(f)]<typename Context>(Context ctx)
    {
        return detail::peek_parser<Context, F>(std::move(f))(std::move(ctx));
    };
};

inline constexpr auto cond = []<typename F>(bool b, F f) static
{
    return [b, f = std::move(f)]<typename Context>(Context ctx)
    {
        return detail::cond_parser<Context, F>(b, std::move(f))(std::move(ctx));
    };
};

inline constexpr auto recognize = []<typename F>(F f) static
{
    return [f = std::move(f)]<typename Context>(Context ctx)
    {
        return detail::recognize_parser<Context, F>(std::move(f))(std::move(ctx));
    };
};

inline constexpr auto rest = []<typename Context>(Context& ctx)
{
    return detail::rest_parser<Context>()(ctx);
};

inline constexpr auto rest_len = []<typename Context>(Context& ctx)
{
    return detail::rest_len_parser<Context>()(ctx);
};

inline constexpr auto not_ = []<typename F>(F f) static
{
    return [f = std::move(f)]<typename Context>(Context ctx)
    {
        return detail::not_parser<Context, F>(std::move(f))(std::move(ctx));
    };
};

inline constexpr auto success = []<typename T>(T v) static
{
    return [v = std::move(v)]<typename Context>(Context ctx)
    {
        return detail::succeed_parser<Context, T>(std::move(v))(std::move(ctx));
    };
};

inline constexpr auto verify = []<typename F, typename Cond>(F f, Cond c) static
{
    return [f = std::move(f), c = std::move(c)]<typename Context>(Context ctx)
    {
        return detail::verify_parser<Context, F, Cond>(std::move(f), std::move(c))(std::move(ctx));
    };
};

inline constexpr auto all_consuming = []<typename F>(F f) static
{
    return [f = std::move(f)]<typename Context>(Context ctx)
    {
        return detail::all_consuming_parser<Context, F>(std::move(f))(std::move(ctx));
    };
};

}  // namespace nom::combinator