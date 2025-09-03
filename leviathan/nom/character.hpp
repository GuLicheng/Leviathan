/*
    Different from rust::nom, our character parsers always return string_view.
    For some parsers which return char(AsChar), we return a string_view of length 1 instead.

    TODO:

    - i8
    - i16
    - i32
    - i64
    - i128
    - isize
    - u8
    - u16
    - u32
    - u64
    - u128
    - usize
*/

#pragma once

#include <leviathan/extc++/chars.hpp>
#include "internal.hpp"

namespace nom::character
{

inline constexpr auto digit0 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop0<Context, decltype(&::isdigit)>(&::isdigit)(std::move(ctx));
};

inline constexpr auto digit1 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop1<Context, decltype(&::isdigit)>(&::isdigit, error_kind::digit)(std::move(ctx));
};

inline constexpr auto alphanumeric0 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop0<Context, decltype(&::isalnum)>(&::isalnum)(std::move(ctx));
};

inline constexpr auto alphanumeric1 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop1<Context, decltype(&::isalnum), error_kind>(&::isalnum, error_kind::alphanumeric)(std::move(ctx));
};

inline constexpr auto alpha0 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop0<Context, decltype(&::isalpha)>(&::isalpha)(std::move(ctx));
};

inline constexpr auto alpha1 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop1<Context, decltype(&::isalpha), error_kind>(&::isalpha, error_kind::alpha)(std::move(ctx));
};

inline constexpr auto space0 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop0<Context, detail::isspace>(detail::isspace())(std::move(ctx));
};
inline constexpr auto space1 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop1<Context, detail::isspace, error_kind>(detail::isspace(), error_kind::space)(std::move(ctx));
};

inline constexpr auto multispace0 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop0<Context, detail::ismultispace>(detail::ismultispace())(std::move(ctx));
};

inline constexpr auto multispace1 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop1<Context, detail::ismultispace, error_kind>(detail::ismultispace(), error_kind::multispace)(std::move(ctx));
};

inline constexpr auto one_of = []<typename StringLike>(StringLike s) static 
{
    return detail::one_of_fn(s);
};

inline constexpr auto none_of = []<typename StringLike>(StringLike s) static 
{
    return detail::none_of_fn(s);
};

inline constexpr auto satisfy = []<typename Pred>(Pred pred) static 
{
    return [pred = std::move(pred)]<typename Context>(Context ctx)  
    {
        return detail::check_first_character<Context, Pred, error_kind>(std::move(pred), error_kind::satisfy)(std::move(ctx));
    };
};

inline constexpr auto char_ = []<typename CharT>(CharT ch) static 
{
    return [=]<typename Context>(Context ctx)  
    {
        auto pred = std::bind_front(std::equal_to<CharT>(), ch);
        return detail::check_first_character<Context, decltype(pred), error_kind>(std::move(pred), error_kind::one_char)(std::move(ctx));
    };
};

inline constexpr auto tab = char_('\t');
inline constexpr auto newline = char_('\n');

inline constexpr auto anychar = []<typename Context>(Context ctx) static 
{
    auto always_true = [](auto&&...) static { return true; };
    return detail::check_first_character<Context, decltype(always_true), error_kind>(always_true, error_kind::eof)(std::move(ctx));
};

inline constexpr auto bin_digit0 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop0<Context, decltype(cpp::isbindigit)>(cpp::isbindigit)(std::move(ctx));
};

inline constexpr auto bin_digit1 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop1<Context, decltype(cpp::isbindigit)>(cpp::isbindigit, error_kind::bin_digit)(std::move(ctx));
};

inline constexpr auto oct_digit0 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop0<Context, decltype(cpp::isoctdighit)>(cpp::isoctdighit)(std::move(ctx));
};

inline constexpr auto oct_digit1 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop1<Context, decltype(cpp::isoctdighit)>(cpp::isoctdighit, error_kind::oct_digit)(std::move(ctx));
};

inline constexpr auto hex_digit0 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop0<Context, decltype(cpp::ishexdigit)>(cpp::ishexdigit)(std::move(ctx));
};

inline constexpr auto hex_digit1 = []<typename Context>(Context ctx) static 
{
    return detail::conditional_loop1<Context, decltype(cpp::ishexdigit)>(cpp::ishexdigit, error_kind::hex_digit)(std::move(ctx));
};

inline constexpr auto crlf = []<typename Context>(Context ctx) static 
{
    return detail::crlf_fn<Context>()(std::move(ctx));
};

inline constexpr auto line_ending = []<typename Context>(Context ctx) static 
{
    return detail::line_ending_fn<Context>()(std::move(ctx));
};

inline constexpr auto not_line_ending = []<typename Context>(Context ctx) static 
{
    return detail::not_line_ending_fn<Context>()(std::move(ctx));
};

}