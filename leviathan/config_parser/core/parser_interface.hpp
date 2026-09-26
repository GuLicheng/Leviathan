#pragma once

#include <type_traits>

namespace cpp::config::parser
{

namespace detail
{

template <typename Parser, typename F> struct map_parser;

}  // namespace detail

struct parser_interface
{
    /**
     * @brief Applies a transformation function to the result of the parser.
     * @details This function allows chaining transformations on the result of a parser, 
     * similar to the `map` function in functional programming languages.
     * The returned result type will be automatically decayed.
     *
     * @tparam Self The type of the parser.
     * @tparam F The type of the transformation function.
     * @param self The parser instance.
     * @param func The transformation function.
     * @return A new parser that applies the transformation function to the result of the original parser.
     */
    template <typename Self, typename F>
    constexpr auto map(this Self&& self, F&& func)
    {
        return detail::map_parser<std::decay_t<Self>, std::decay_t<F>>{ (Self&&) self, (F&&) func };
    }
};

}  // namespace cpp::config::parser




