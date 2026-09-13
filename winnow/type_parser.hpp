#pragma once

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
        else if (stream.match("0", true))
        {
            base = 8;
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
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        // () ( )
        // (1, 2, 3) 
        using E = typename Stream::error_type;
        using O = TupleLike;
        using R = modal_result<O, E>;

        auto left = token::literal("(");
        auto right = token::literal(")");
        
        throw std::runtime_error("tuple_parser not implemented");
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

        auto left = combinator::preceded(token::literal("["), ascii::multispace0);
        auto right = combinator::terminated(ascii::multispace0, token::literal("]"));
        auto separator = combinator::delimited(ascii::multispace0, token::literal(","), ascii::multispace0);
        auto middle = combinator::separated<std::vector>(universal_parser<V>(), separator);
        
        auto parser = combinator::delimited(left, middle, right);
        // std::ranges::copy

        auto result = parser(stream);

        if (!result)
        {
            return make_backtrack_from_input<R>(stream);
        }

        return R(std::in_place, Range(std::from_range, std::move(result.value())));
    }
};

template <typename T>
struct universal_parser
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
