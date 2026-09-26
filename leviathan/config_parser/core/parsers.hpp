/*

    combinators
        - map
        - sequence
        - preceded
        - terminated
        - delimited
        - separated_pair

    token
        - take_while
        - literal
    
    ascii
        - alpha0
        - alpha1
        - digit0
        - digit1
        - alphanumeric0
        - alphanumeric1
        - space0
        - space1
        - hexdigit0
        - hexdigit1


    - [x] take_till
    - [x] take
    - [x] take_until
    - [x] rest
    - [x] rest_len
    - [x] any
    - [x] none_of
    - [x] one_of

    - [x] alt
    - [x] backtrack_err
    - [x] cond
    - [x] cut_err
    - [x] empty
    - [x] eof
    - [x] expression
    - [x] fail
    - [x] fill
    - [x] iterator
    - [x] not
    - [x] opt
    - [x] peek
    - [x] repeat
    - [x] repeat_till
    - [x] separated
    - [x] separated_foldl1
    - [x] separated_foldr1
    - [x] todo
    - [x] trace

    - [x] oct_digit0
    - [x] oct_digit1
    - [x] multispace0
    - [x] multispace1
    - [x] newline
    - [x] tab
    - [x] crlf
    - [x] till_line_ending
    - [x] line_ending
    - [x] line_comment
    - [x] block_comment
    - [x] escaped
    
    - [x] dec_int
    - [x] dec_uint
    - [x] float
    - [x] hex_uint
    - [x] take_escaped


    - [x] void
    - [x] value
    - [x] cut
    - [x] and_then
    - [x] context
    - [x] verify


    - parse : operator()
    - [x] parse_iter
    - [x] parse_peek
    - [x] by_ref           -> std::ref/std::cref is OK
    - [x] default_value    -> <T>.map([](auto&&) { return T(); })
    - [x] output_into
    - [x] take
    - [x] with_take
    - [x] span
    - [x] span
    - [x] with_span
    - [x] try_map
    - [x] verify_map
    - [x] flat_map
    - [x] parse_to         -> .map(std::str::FromStr)
    - [x] context_with
    - [x] map_err
    - [x] complete_err
    - [x] err_into
    - [x] retry_after
    - [x] resume_after

*/

#pragma once

#include <leviathan/extc++/tuple.hpp>
#include <leviathan/config_parser/core/internal.hpp>
#include <leviathan/config_parser/core/parser_interface.hpp>
#include <type_traits>

namespace cpp::config::parser
{

inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred&& pred, occurrences<size_t> range) 
    {
        return detail::take_while_parser<std::decay_t<Pred>>((Pred&&) pred, range);
    }
} take_while;

inline constexpr auto alpha0 = take_while(::isalpha, { 0, std::nullopt });
inline constexpr auto alpha1 = take_while(::isalpha, { 1, std::nullopt });

inline constexpr auto digit0 = take_while(::isdigit, { 0, std::nullopt });
inline constexpr auto digit1 = take_while(::isdigit, { 1, std::nullopt });

inline constexpr auto alphanumeric0 = take_while(::isalnum, { 0, std::nullopt });
inline constexpr auto alphanumeric1 = take_while(::isalnum, { 1, std::nullopt });

inline constexpr auto space0 = take_while(::isspace, { 0, std::nullopt });
inline constexpr auto space1 = take_while(::isspace, { 1, std::nullopt });

inline constexpr auto hexdigit0 = take_while(::isxdigit, { 0, std::nullopt });
inline constexpr auto hexdigit1 = take_while(::isxdigit, { 1, std::nullopt });

// If we want to use the combinators in a more functional style, 
// we can use these functions instead of the parser_interface methods.
inline constexpr struct
{
    template <typename Parser, typename F>
    static constexpr auto operator()(Parser&& parser, F&& f)
    {
        return detail::map_parser<std::decay_t<Parser>, std::decay_t<F>>((Parser&&) parser, (F&&) f);
    }
} map;

inline constexpr struct
{
    template <typename... Parsers>
    static constexpr auto operator()(Parsers&&... parsers)
    {
        return detail::sequence_parser<std::decay_t<Parsers>...>((Parsers&&) parsers...);
    }
} sequence;
    
inline constexpr struct
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1&& f1, F2&& f2)
    {
        return sequence((F1&&) f1, (F2&&) f2).map(cpp::elements<1>);
    }
} preceded;

inline constexpr struct
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1&& f1, F2&& f2)
    {
        return sequence((F1&&) f1, (F2&&) f2).map(cpp::elements<0>);
    }
} terminated;

inline constexpr struct
{
    template <typename F1, typename F2, typename F3>
    static constexpr auto operator()(F1&& f1, F2&& f2, F3&& f3)
    {
        return sequence((F1&&) f1, (F2&&) f2, (F3&&) f3).map(cpp::elements<1>);
    }
} delimited;

inline constexpr struct
{
    template <typename F1, typename F2, typename F3>
    static constexpr auto operator()(F1&& f1, F2&& f2, F3&& f3)
    {
        return sequence((F1&&) f1, (F2&&) f2, (F3&&) f3).map(cpp::select_tuple_elements<0, 2>);
    }
} separated_pair;

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> str)
    {
        return detail::literal_parser<CharT>(str);
    }

    template <typename CharT>
    static constexpr auto operator()(const CharT* c)
    {
        std::basic_string_view<CharT> sv(c);
        return detail::literal_parser<CharT>(sv);
    }
} literal;

}  // namespace cpp::config::parser
