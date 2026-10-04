/*
    Some people, when confronted with a problem, think “I know, 
    I’ll use regular expressions.” Now they have two problems.
    
                                                -------- Jamie Zawinski

    Follow parsers are copied from winnow.(See winnow/winnow.hpp)

        - map
        - sequence
        - preceded
        - terminated
        - delimited
        - separated_pair
        - alt
        - map_err
        - value
        - default_value    
        - repeat
        - take_while
        - literal
        - alpha0
        - alpha1
        - digit0
        - digit1
        - alphanumeric0
        - alphanumeric1
        - multispace0
        - multispace1
        - hexdigit0
        - hexdigit1
        - newline
        - tab
        - crlf
        - take_until
        - take_till
        - number | dec_int | dec_uint | float | hex_uint
        - recoverable | backtrack_err
        - fatal | cut_err
        - till_line_ending
        - line_ending
        - line_comment
        - block_comment
        - verify
        - and_then
        - take
        - any
        - none_of
        - one_of
        - empty
        - eof
        - peek
        - separated_foldl1
        - take
        - repeat_till
        - cond
        - opt
        - not


    - [x] rest
    - [x] rest_len

    - [x] expression
    - [x] fail
    - [x] fill
    - [x] iterator
    - [x] separated
    - [x] separated_foldr1
    - [x] todo
    - [x] trace

    - [x] oct_digit0
    - [x] oct_digit1
    - [x] escaped
    - [x] take_escaped
    - [x] void
    - [x] context

    - parse : operator()
    - [x] parse_iter
    - [x] parse_peek
    - [x] by_ref           -> std::ref/std::cref is OK
    - [x] output_into
    - [x] with_take
    - [x] span
    - [x] span
    - [x] with_span
    - [x] try_map
    - [x] verify_map
    - [x] flat_map
    - [x] parse_to         -> .map(std::str::FromStr)
    - [x] context_with
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

/**
 * @brief Wraps a callable into a parser interface.
 * 
 * @details This parser takes a callable (e.g., a lambda or function object) 
 * and wraps it into a parser interface, allowing it to be used seamlessly with other parsers.
 */
inline constexpr struct
{
    template <typename Callable>
    static constexpr auto operator()(Callable&& parser)
    {
        return detail::warpper_parser<std::decay_t<Callable>>((Callable&&) parser);
    }
} as_parser;

inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred&& pred, occurrences<size_t> range) 
    {
        return detail::take_while_parser<std::decay_t<Pred>>((Pred&&) pred, range);
    }
} take_while;

inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred&& pred, occurrences<size_t> range) 
    {
        return take_while(std::not_fn((Pred&&) pred), range);
    }
} take_till;

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> value, occurrences<size_t> range)
    {
        return detail::take_until_parser<CharT>(value, range);
    }
    
    template <typename CharT>
    static constexpr auto operator()(const CharT* value, occurrences<size_t> range)
    {
        std::basic_string_view<CharT> sv(value);
        return operator()(sv, range);
    }
} take_until;

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
        return operator()(sv);
    }
} literal;

inline constexpr struct
{
    template <typename... Parsers>
    static constexpr auto operator()(Parsers&&... parsers)
    {
        return detail::alternative_parser<std::decay_t<Parsers>...>((Parsers&&) parsers...);
    }
} alt;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser&& parser, occurrences<size_t> range)
    {
        return detail::repeat_parser<std::decay_t<Parser>>((Parser&&) parser, range);
    }
} repeat;

template <typename T>
inline constexpr auto number = detail::int_parser<T>();

template <std::floating_point T>
inline constexpr auto number<T> = detail::float_parser<T>();

inline constexpr auto alpha0 = take_while(::isalpha, { 0, std::nullopt });
inline constexpr auto alpha1 = take_while(::isalpha, { 1, std::nullopt });

inline constexpr auto digit0 = take_while(::isdigit, { 0, std::nullopt });
inline constexpr auto digit1 = take_while(::isdigit, { 1, std::nullopt });

inline constexpr auto alphanumeric0 = take_while(::isalnum, { 0, std::nullopt });
inline constexpr auto alphanumeric1 = take_while(::isalnum, { 1, std::nullopt });

inline constexpr auto multispace0 = take_while(::isspace, { 0, std::nullopt });
inline constexpr auto multispace1 = take_while(::isspace, { 1, std::nullopt });

inline constexpr auto hexdigit0 = take_while(::isxdigit, { 0, std::nullopt });
inline constexpr auto hexdigit1 = take_while(::isxdigit, { 1, std::nullopt });

inline constexpr auto newline = literal("\n");
inline constexpr auto tab = literal("\t");
inline constexpr auto crlf = literal("\r\n");
inline constexpr auto line_ending = alt(crlf, newline);
inline constexpr auto till_line_ending = detail::till_line_ending_parser<char>();

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
        return sequence(
            literal(start), 
            take_until(finish, { 0, std::nullopt }), 
            literal(finish)
        );
    }
} block_comment;

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
        return sequence(literal(sv), till_line_ending);
    }
} line_comment; 

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser&& parser)
    {
        return detail::peek_parser<std::decay_t<Parser>>((Parser&&) parser);
    }
} peek;

inline constexpr struct 
{
    static constexpr auto operator()(size_t count)
    {
        return detail::take_parser(count);
    }
} take;

inline constexpr auto any = detail::check_next_character_parser<detail::always_false>(detail::always_false());

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
        auto contains = [=](auto c) { return std::ranges::contains(str, c); };
        return detail::check_next_character_parser<decltype(contains)>(std::move(contains));
    }
} none_of;

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
        auto contains = [=](auto c) { return !std::ranges::contains(str, c); };
        return detail::check_next_character_parser<decltype(contains)>(std::move(contains));
    }
} one_of;

inline constexpr auto eof = detail::eof_parser();

inline constexpr auto empty = detail::empty_parser();

inline constexpr struct
{
    template <typename Parser, typename Seperator, typename BinaryOp>
    static constexpr auto operator()(Parser&& p, Seperator&& sep, BinaryOp&& binop)
    {
        return detail::separated_foldl1_parser<
            std::decay_t<Parser>, std::decay_t<Seperator>, std::decay_t<BinaryOp>
        >((Parser&&) p, (Seperator&&) sep, (BinaryOp&&) binop);
    }
} separated_foldl1;

inline constexpr struct
{
    template <typename Parser, typename TerminatorParser>
    static constexpr auto operator()(Parser&& parser, TerminatorParser&& terminator_parser, occurrences<size_t> r)
    {
        return detail::repeat_till_parser<
            std::decay_t<Parser>, std::decay_t<TerminatorParser>
        >((Parser&&) parser, (TerminatorParser&&) terminator_parser, r);
    }
} repeat_till;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(bool condition, Parser&& parser)
    {
        return detail::cond_parser<std::decay_t<Parser>>(condition, (Parser&&) parser);
    }
} cond;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser&& parser)
    {
        return detail::opt_parser<std::decay_t<Parser>>((Parser&&) parser);
    }
} opt;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser&& parser)
    {
        return detail::not_parser<std::decay_t<Parser>>((Parser&&) parser);
    }
} not_;

}  // namespace cpp::config::parser
