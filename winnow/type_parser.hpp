#pragma once

#include <leviathan/extc++/meta.hpp>

#include <ranges>
#include <type_traits>
#include <concepts>

#include "utils.hpp"
#include "error.hpp"
#include "result.hpp"
#include "token.hpp"
#include "combinator.hpp"
#include "ascii.hpp"

namespace winnow
{

template <typename T>
struct universal_parser;

template <std::integral Integral>
struct int_parser
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using O = Integral;
        using R = modal_result<O, E>;

        int base = 10;

        if (stream.match("0b", true))
        {
            base = 2;
        }
        else if (stream.match("0x", true))
        {
            base = 16;
        }

        Integral result;
        auto [ptr, ec] = std::from_chars(stream.begin(), stream.end(), result, base);

        // Check if parsing was successful
        if (ec == std::errc())
        {
            // Successfully parsed the number
            // Advance stream
            stream.advance(ptr - stream.begin());
            return R(std::in_place, result);
        }

        // Failed to parse the number
        return make_backtrack_from_input<R>(stream);
    }
};

template <std::floating_point Floating>
struct float_parser
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using O = Floating;
        using R = modal_result<O, E>;

        Floating result;
        auto [ptr, ec] = std::from_chars(stream.begin(), stream.end(), result);

        // Check if parsing was successful
        if (ec == std::errc())
        {
            // Successfully parsed the number
            // Advance stream
            stream.advance(ptr - stream.begin());
            return R(std::in_place, result);
        }

        // Failed to parse the number
        return make_backtrack_from_input<R>(stream);
    }
};

struct boolean_parser
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using O = bool;
        using R = modal_result<O, E>;

        if (stream.match("true", true) || stream.match("True", true))
        {
            return R(std::in_place, true);
        }
        else if (stream.match("false", true) || stream.match("False", true))
        {
            return R(std::in_place, false);
        }

        return make_backtrack_from_input<R>(stream);
    }
};

template <cpp::meta::tuple_like TupleLike>
struct tuple_parser
{
    static constexpr auto size = std::tuple_size_v<TupleLike>;

    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        // () ( )
        // (1, 2, 3) 
        using E = typename Stream::error_type;
        using O = TupleLike;
        using R = modal_result<O, E>;

        auto left = combinator::sequence(token::literal("("), ascii::multispace0);
        auto right = combinator::sequence(ascii::multispace0, token::literal(")"));

        if constexpr (size == 0)
        {
            auto result = combinator::sequence(left, right);
            return result ? R(std::in_place) : make_backtrack_from_input<R>(stream);
        }
        else
        {
            auto separator = combinator::delimited(
                ascii::multispace0,
                token::literal(","),
                ascii::multispace0
            );

            constexpr auto [...idx] = std::make_index_sequence<size - 1>();

            auto middle = combinator::sequence(
                universal_parser<std::tuple_element_t<0, TupleLike>>(),
                combinator::preceded(separator, universal_parser<std::tuple_element_t<idx + 1, TupleLike>>())...
            );

            auto allow_trailing = combinator::terminated(
                middle,
                combinator::cond(true, combinator::opt(separator))
            );

            auto parser = combinator::delimited(
                left,
                allow_trailing,
                right
            );

            return parser(stream);
        }
    }
};

template <std::ranges::range Range>
struct range_parser
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using O = Range;
        using V = std::ranges::range_value_t<Range>;
        using R = modal_result<O, E>;

        // [] [ ]
        // [1, 2, 3]

        auto seperator = combinator::delimited(
            ascii::multispace0, 
            token::literal(","), 
            ascii::multispace0
        );

        auto values = combinator::separated<Range>(
            universal_parser<V>(), 
            seperator,
            from(0)        
        );

        auto allow_trailing = combinator::terminated(
            values, 
            combinator::cond(true, combinator::opt(seperator))
        );

        auto parser = combinator::sequence(
            combinator::preceded(
                token::literal("["), 
                ascii::multispace0
            ),
            allow_trailing,
            combinator::terminated(
                ascii::multispace0,
                token::literal("]")
            )
        );

        auto result = parser(stream);

        return !result 
             ? make_backtrack_from_input<R>(stream) 
             : R(std::in_place, std::move(std::get<1>(result.value())));
    }
};

template <typename Enum>
struct enum_parser;

template <typename T>
struct aggregate_parser;

template <typename T>
struct universal_parser : detail::parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using O = T;
        using R = modal_result<O, E>;

        if constexpr (std::is_same_v<T, bool>)
        {
            return boolean_parser{}(stream);
        }
        else if constexpr (std::integral<T>)
        {
            return int_parser<T>{}(stream);
        }
        else if constexpr (std::floating_point<T>)
        {
            return float_parser<T>{}(stream);
        }
        else if constexpr (std::ranges::range<T>)
        {
            return range_parser<T>{}(stream);
        }
        else if constexpr (cpp::tuple_like<T>)
        {
            return tuple_parser<T>{}(stream);
        }
        else
        {
            static_assert(false, "No parser available for this type");
            // return universal_parser<T>{}(stream);
        }
    }
};

template <typename T>
inline constexpr universal_parser<T> universal{};

}  // namespace winnow
