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
    
    - [x] dec_int
    - [x] dec_uint
    - [x] escaped
    - [x] float
    - [x] hex_uint
    - [x] line_ending
    - [x] take_escaped
    - [x] till_line_ending
*/

#pragma once

#include "token.hpp"

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



} // namespace winnow::ascii
