/*
    https://docs.rs/winnow/latest/winnow/combinator/index.html

    - alt
    - backtrack_err
    - cond
    - cut_err
    - delimited
    - empty
    - eof
    - [x] expression
    - fail
    - fill
    - iterator
    - not
    - opt
    - peek
    - preceded
    - repeat
    - repeat_till
    - separated
    - [x] separated_foldl1
    - [x] separated_foldr1
    - separated_pair
    - terminated
    - [x] todo
    - [x] trace

*/

#pragma once

#include <leviathan/extc++/tuple.hpp>
#include "internal.hpp"

namespace winnow::combinator
{

// If we want to use the combinators in a more functional style, 
// we can use these functions instead of the parser_interface methods.
inline constexpr struct
{
    template <typename Parser, typename F>
    static constexpr auto operator()(Parser parser, F f)
    {
        return detail::map_parser<Parser, F>(std::move(parser), std::move(f));
    }
} map;

inline constexpr struct
{
    template <typename... Parsers>
    static constexpr auto operator()(Parsers... parsers)
    {
        return detail::sequence_parser<Parsers...>(std::move(parsers)...);
    }
} sequence;
    
inline constexpr struct
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1 f1, F2 f2)
    {
        auto second_element = []<typename Tuple>(Tuple&& tuple) static {
            return std::get<1>((Tuple&&)tuple);
        };
        return sequence(std::move(f1), std::move(f2)).map(second_element);
    }
} preceded;

inline constexpr struct
{
    template <typename F1, typename F2>
    static constexpr auto operator()(F1 f1, F2 f2)
    {
        auto first_element = []<typename Tuple>(Tuple&& tuple) static {
            return std::get<0>((Tuple&&)tuple);
        };
        return sequence(std::move(f1), std::move(f2)).map(first_element);
    }
} terminated;

inline constexpr struct
{
    template <typename F1, typename F2, typename F3>
    static constexpr auto operator()(F1 f1, F2 f2, F3 f3)
    {
        auto second_element = []<typename Tuple>(Tuple&& tuple) static {
            return std::get<1>((Tuple&&)tuple);
        };
        return sequence(std::move(f1), std::move(f2), std::move(f3)).map(second_element);
    }
} delimited;

inline constexpr struct
{
    template <typename F1, typename F2, typename F3>
    static constexpr auto operator()(F1 f1, F2 f2, F3 f3)
    {
        auto first_and_third = []<typename Tuple>(Tuple&& tuple) static {
            return std::make_pair(std::get<0>((Tuple&&)tuple), std::get<2>((Tuple&&)tuple));
        };
        return sequence(std::move(f1), std::move(f2), std::move(f3)).map(first_and_third);
    }
} separated_pair;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::backtrack_err_parser<Parser>(std::move(parser));
    }
} backtrack_err;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::cut_err_parser<Parser>(std::move(parser));
    }
} cut_err;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::peek_parser<Parser>(std::move(parser));
    }
} peek;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::opt_parser<Parser>(std::move(parser));
    }
} opt;

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(Parser parser)
    {
        return detail::not_parser<Parser>(std::move(parser));
    }
} not_;

inline constexpr struct
{
    template <typename... Parsers>
    static constexpr auto operator()(Parsers... parsers)
    {
        return detail::choice_parser<Parsers...>(std::move(parsers)...);
    }
} alt;

// Maybe we have to offer two functions instead of an object since we need
// specified container type
template <template <typename...> typename Container, typename Parser, typename Sep, typename... Args>
constexpr auto separated(Parser parser, Sep separator, occurrences<size_t> range, Args&&... args)
{
    return detail::separated_container_parser<Parser, Sep, Container, std::decay_t<Args>...>
        ((Parser&&)parser, (Sep&&)separator, range, (Args&&)args...);
}

template <typename Container, typename Parser, typename Sep, typename... Args>
constexpr auto separated(Parser parser, Sep separator, occurrences<size_t> range, Args&&... args)
{
    return detail::separated_parser<Parser, Sep, accumulate_traits<Container>, std::decay_t<Args>...>
        ((Parser&&)parser, (Sep&&)separator, accumulate_traits<Container>(), range, (Args&&)args...);
}

inline constexpr struct
{
    template <typename Parser>
    static constexpr auto operator()(bool condition, Parser parser)
    {
        return detail::cond_parser<Parser>(condition, std::move(parser));
    }
} cond;

inline constexpr detail::empty_parser empty;

template <typename Output>
inline constexpr detail::fail_fn<Output> fail;

inline constexpr detail::eof_parser eof;

inline constexpr struct
{
    template <typename Stream, typename Parser>
    static constexpr auto operator()(Stream& stream, Parser parser)
    {
        return detail::iterator_parser<Stream, Parser>(stream, std::move(parser));
    }
} iterator;

inline constexpr struct
{
    template <typename Parser, typename Iterator, typename Sentinel>
    static constexpr auto operator()(Parser parser, Iterator iter, Sentinel sent)
    {
        return detail::fill_parser<Parser, Iterator, Sentinel>(std::move(parser), std::move(iter), std::move(sent));
    }
} fill;

inline constexpr struct
{
    template <typename Parser, typename TerminatorParser, typename Accumulator>
    static constexpr auto operator()(Parser parser, TerminatorParser terminator_parser, Accumulator accumulator, size_t lower = 0, std::optional<size_t> upper = std::nullopt)
    {
        return detail::repeat_till_parser<Parser, TerminatorParser, Accumulator>(std::move(parser), std::move(terminator_parser), std::move(accumulator), occurrences<size_t>(lower, upper));
    }
} repeat_till;

template <template <typename...> class Container>
inline constexpr detail::repeat_fn<Container> repeat;


}  // namespace winnow::combinator
