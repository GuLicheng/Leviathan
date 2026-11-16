/*
    https://docs.rs/winnow/latest/winnow/ascii/index.html

    - alpha0
    - alpha1
    - digit0
    - digit1
    - alphanumeric0
    - alphanumeric1
    - space0
    - space1
    - hex_digit0
    - hex_digit1
    - oct_digit0
    - oct_digit1
    - multispace0
    - multispace1
    - newline
    - tab
    - crlf
    - till_line_ending
    - line_ending
    - line_comment
    - block_comment
    - escaped
    
    - [x] dec_int
    - [x] dec_uint
    - [x] float
    - [x] hex_uint
    - [x] take_escaped

    We offer universal parsers for various types, such as integers, floating-point numbers, and ranges.
*/

#pragma once

#include "token.hpp"
#include "combinator.hpp"

namespace winnow::ascii
{

inline constexpr auto alpha0 = token::take_while(::isalpha, 0);
inline constexpr auto alpha1 = token::take_while(::isalpha, 1);

inline constexpr auto digit0 = token::take_while(::isdigit, 0);
inline constexpr auto digit1 = token::take_while(::isdigit, 1);

inline constexpr auto alphanumeric0 = token::take_while(::isalnum, 0);
inline constexpr auto alphanumeric1 = token::take_while(::isalnum, 1);

inline constexpr auto space0 = token::take_while([](char c) { return c == ' ' || c == '\t'; }, 0);
inline constexpr auto space1 = token::take_while([](char c) { return c == ' ' || c == '\t'; }, 1);

inline constexpr auto hex_digit0 = token::take_while([](char c) { return ::isxdigit(c); }, 0);
inline constexpr auto hex_digit1 = token::take_while([](char c) { return ::isxdigit(c); }, 1);

inline constexpr auto oct_digit0 = token::take_while([](char c) { return c >= '0' && c <= '7'; }, 0);
inline constexpr auto oct_digit1 = token::take_while([](char c) { return c >= '0' && c <= '7'; }, 1);

inline constexpr auto multispace0 = token::take_while([](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }, 0);
inline constexpr auto multispace1 = token::take_while([](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }, 1);

inline constexpr auto newline = token::literal("\n");
inline constexpr auto tab = token::literal("\t");
inline constexpr auto crlf = token::literal("\r\n");
inline constexpr auto line_ending = combinator::alt(crlf, newline);
inline constexpr auto till_line_ending = detail::till_line_ending_parser<char>();

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* start) 
    {
        return operator()(std::basic_string_view<CharT>(start));
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> sv) 
    {
        return combinator::sequence(token::literal(sv), till_line_ending);
    }
} line_comment; 

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* start, const CharT* finish) 
    {
        return operator()(
            std::basic_string_view<CharT>(start),
            std::basic_string_view<CharT>(finish)
        );
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> start, std::basic_string_view<CharT> finish) 
    {
        return combinator::sequence(
            token::literal(start), 
            token::take_until(finish), 
            token::literal(finish)
        );
    }
} block_comment;

inline constexpr struct
{
    template <typename Normal, typename ControlChar, typename Escape>
    static constexpr auto operator()(Normal normal_parser, ControlChar control_char, Escape escape_parser)
    {
        return detail::escaped_parser(std::move(normal_parser), std::move(control_char), std::move(escape_parser));
    }
} escaped;

} // namespace winnow::ascii
