/*
    1. Each scanner should derived from context_interface
    2. We assume that if parsing fails, the context state is indeterminate.
    3. Users should ensure that all supplied parsers have non-`void` return types.
*/

#pragma once

#include <leviathan/extc++/tuple.hpp>

#include <cctype>
#include <string_view>
#include <optional>
#include <charconv>
#include <algorithm>
#include <ranges>
#include <concepts>
#include <meta>
#include <functional>

namespace cpp::config::scanner
{

template <typename F>
struct conditional_loop
{
    [[no_unique_address]] F func;

    template <typename F2>
    constexpr conditional_loop(F2&& f2) : func((F2&&) f2) { }

    template <typename Context>
    constexpr auto operator()(Context& ctx) const
    {
        const auto sv = ctx.to_string_view();
        const size_t rest = ctx.size();
        for (; !ctx.eof() && std::invoke(func, ctx.current()); ctx.advance(1));
        return sv.substr(0, rest - ctx.size());
    }
};

template <typename F>
conditional_loop(F) -> conditional_loop<F>;

template <typename... Parsers>
struct sequence
{
    static constexpr auto indices = std::make_index_sequence<sizeof...(Parsers)>{};
    
    [[no_unique_address]] std::tuple<Parsers...> parsers;

    template <typename... Parsers2>
    constexpr sequence(Parsers2&&... parsers2) : parsers((Parsers2&&) parsers2...) { }

    template <typename Context>
        requires (std::invocable<Parsers, Context&> && ...)
    constexpr auto operator()(Context& ctx) const
    {
        using T = Context&;
        using RT = typename [:calculate_return_type(^^T):];
        constexpr auto [...idx] = indices;
        // RT(...) is avoided because argument evaluation order 
        // is unspecified (not guaranteed left-to-right).
        // Using RT{...} ensures that the parsers are evaluated
        // in the correct order. [dcl.init.list] §4
        return RT{ eval_one<RT, idx>(ctx)... };
    }

private:

    template <typename RT, std::size_t I, typename Context>
    constexpr auto eval_one(Context& ctx) const  
    {
        std::println("Evaluating parser at index {}", I);
        return std::invoke(std::get<I>(parsers), ctx);
        // using Elem = std::tuple_element_t<I, RT>;
        // if constexpr (std::same_as<Elem, std::nullptr_t>) 
        // {
        //     std::invoke(std::get<I>(parsers), ctx); // void parser
        //     return nullptr;
        // } 
        // else 
        // {
        //     return std::invoke(std::get<I>(parsers), ctx);
        // }
    }

    static consteval std::meta::info calculate_return_type(std::meta::info ctx)
    {
        auto void_to_null = [=](auto info) { 
            auto ret = std::meta::invoke_result(info, { ctx });
            return is_void_type(ret) ? ^^std::nullptr_t : ret; 
        };
        auto args = std::vector { dealias(^^Parsers)... }
                  | std::views::transform(void_to_null)
                  | std::ranges::to<std::vector>();
        return std::meta::substitute( ^^cpp::tuple, args );
    }
};

template <typename... Parsers>
sequence(Parsers&&...) -> sequence<Parsers...>;

// template <typename... Parsers>
// struct alternative
// {
//     static_assert(sizeof...(Parsers) > 0, "alternative requires at least one parser");

//     static constexpr auto indices = std::make_index_sequence<sizeof...(Parsers)>{};
    
//     [[no_unique_address]] std::tuple<Parsers...> parsers;

//     template <typename... Parsers2>
//     constexpr alternative(Parsers2&&... parsers2) : parsers((Parsers2&&) parsers2...) { }

//     template <typename Context>
//     constexpr auto operator()(Context& ctx) const
//     {
//         using R1 = std::invoke_result_t<Parsers...[0], Context&>;
//         using R = std::optional<R1>;
//         static_assert(std::is_convertible_v<R, bool>, "The return type of the first parser must be convertible to bool");

//         R result;

//         template for (const auto& parser : parsers)
//         {
//             auto result = parser(ctx);

//             if (result)
//             {
//                 break;
//             }
//         }

//         return result;
//     }
// };

// template <typename... Parsers>
// alternative(Parsers&&...) -> alternative<Parsers...>;

inline constexpr auto skip_whitespace = conditional_loop(::isspace);
inline constexpr auto alpha = conditional_loop(::isalpha);
inline constexpr auto digit = conditional_loop(::isdigit);
inline constexpr auto alphanumeric = conditional_loop(::isalnum);

template <typename CharT> 
struct literal
{
    std::basic_string_view<CharT> value;

    constexpr literal(std::basic_string_view<CharT> value) : value(value) { }
    
    constexpr literal(const CharT* value) : value(value) { }

    template <typename Context>
    constexpr bool operator()(Context& ctx) const
    {
        return ctx.match(value, true);
    }
};

template <typename T> struct parse;

template <> 
struct parse<bool>
{
    static constexpr std::string_view True = "true";

    static constexpr std::string_view False = "false";

    bool allow_bool_case_insensitive;

    constexpr parse(bool allow_bool_case_insensitive = false) 
        : allow_bool_case_insensitive(allow_bool_case_insensitive) { }

    template <typename Context>
    constexpr std::optional<bool> operator()(Context& ctx) const
    {
        if (!allow_bool_case_insensitive)
        {
            if (ctx.match(True, true))
            {
                return std::make_optional(true);
            }

            if (ctx.match(False, true))
            {
                return std::make_optional(false);
            }
        }
        else
        {
            if (std::ranges::equal(ctx.to_string_view().substr(0, 4), True, {}, ::tolower, ::tolower))
            {
                ctx.advance(4);
                return std::make_optional(true);
            }

            if (std::ranges::equal(ctx.to_string_view().substr(0, 5), False, {}, ::tolower, ::tolower))
            {
                ctx.advance(5);
                return std::make_optional(false);
            }
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