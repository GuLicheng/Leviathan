/*
    1. Each scanner should derived from context_interface
    2. We assume when parsing fails, the state of context is indeterminate.
*/

#pragma once

#include <cctype>
#include <optional>
#include <charconv>

namespace cpp::config::scanner
{

template <typename F>
struct conditional_loop
{
    [[no_unique_address]] F func;

    template <typename F2>
    constexpr conditional_loop(F2&& f2) : func((F2&&) f2) { }

    template <typename Context>
    constexpr size_t operator()(Context& ctx)
    {
        const size_t rest = ctx.size();
        for (; !ctx.eof() && func(ctx.current()); ctx.advance(1));
        return rest - ctx.size();
    }
};

template <typename F>
conditional_loop(F) -> conditional_loop<F>;

inline constexpr auto skip_whitespace = conditional_loop(::isspace);
inline constexpr auto alpha = conditional_loop(::isalpha);
inline constexpr auto digit = conditional_loop(::isdigit);
inline constexpr auto alphanumeric = conditional_loop(::isalnum);

template <typename T> struct parse;

template <> 
struct parse<bool>
{
    template <typename Context>
    static constexpr std::optional<bool> operator()(Context& ctx)
    {
        if (ctx.match("true", true) || ctx.match("True", true))
        {
            return std::make_optional(true);
        }
        
        if (ctx.match("false", true) || ctx.match("False", true))
        {
            return std::make_optional(false);
        }
        
        return std::nullopt;
    }
};

template <typename Arithmetic> 
    requires std::is_arithmetic_v<Arithmetic>
struct parse<Arithmetic>
{
    template <typename Context>
    static constexpr std::optional<Arithmetic> operator()(Context& ctx)
    {
        Arithmetic result;
        auto [ptr, ec] = std::from_chars(ctx.data(), ctx.data() + ctx.size(), result);

        if (ec != std::errc())
        {
            return std::nullopt;
        }

        auto consumed = ptr - ctx.data();
        ctx.advance(consumed);
        return std::make_optional(result);
    }
};





}  // namespace cpp::config::scanner