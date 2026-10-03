#pragma once

#include <type_traits>
#include <leviathan/config_parser/core/error.hpp>

namespace cpp::config::parser
{

namespace detail
{

template <typename Parser, typename F> class map_parser;

// template <typename Parser, typename F> class map_err_parser;

template <typename Parser, typename F> class map_result_parser;

template <typename Parser> class warpper_parser;

}  // namespace detail

struct parser_interface
{
    /**
     * @brief Marks the result as recoverable.
     */
    template <typename Self>
    constexpr auto recoverable(this Self&& self)
    {
        auto fn = []<typename T>(T&& result) static 
        {
            if (!result)
            {
                result.error().switch_to_recoverable();
            }
            return ((T&&) result);
        };
        return detail::map_result_parser<std::decay_t<Self>, decltype(fn)>{ (Self&&) self, fn };
    }

    /**
     * @brief Marks the result as fatal.
     */
    template <typename Self>
    constexpr auto fatal(this Self&& self)
    {
        auto fn = []<typename T>(T&& result) static 
        {
            if (!result)
            {
                result.error().switch_to_fatal();
            }
            return ((T&&) result);
        };
        return detail::map_result_parser<std::decay_t<Self>, decltype(fn)>{ (Self&&) self, fn };
    }

    /**
     * @brief Replaces the result of the parser with the default value of type `T`.
     * @details This function allows chaining a default value to the result of a parser,
     * similar to the `default_value` function in functional programming languages.
     * The returned result type will be automatically decayed.
     * 
     * @tparam T The type of the default value.
     * @tparam Self The type of the parser.
     * @param self The parser instance.
     * @return A new parser that replaces the result of the original parser with the default value of type `T`.
     */
    template <typename T, typename Self>
    constexpr auto default_value(this Self&& self)
    {
        return self.map([](auto&&) static { return T{}; });
    }

    /**
     * @brief Applies a transformation function to the error result of the parser.
     * @details This function allows chaining transformations on the error result of a parser,
     * similar to the `map_err` function in functional programming languages.
     * The returned result type will be automatically decayed.
     *
     * @tparam Self The type of the parser.
     * @tparam F The type of the transformation function.
     * @param self The parser instance.
     * @param func The transformation function.
     * @return A new parser that applies the transformation function to the error result of the original parser.
     */
    template <typename Self, typename F>
    constexpr auto map_err(this Self&& self, F&& func)
    {
        auto fn = [func = (F&&) func]<typename T>(T&& result)
        {
            // return ((T&&) result).error().transform(func);
            using R1 = std::decay_t<T>;
            using E1 = typename R1::error_type;
            using E = std::decay_t<std::invoke_result_t<F, E1>>;
            using O = typename R1::value_type;
            using R = parse_result<O, E>;

            return result ? R(std::in_place, std::move(result.value()))
                          : R(std::unexpect, result.error().is_recoverable(), std::invoke(func, std::move(result.error())));
        };
        return detail::map_result_parser<std::decay_t<Self>, decltype(fn)>{ (Self&&) self, std::move(fn) };
    }

    /**
     * @brief Replaces the result of the parser with a fixed value.
     * @details This function allows chaining a fixed value to the result of a parser,
     * similar to the `value` function in functional programming languages.
     * The returned result type will be automatically decayed.
     *
     * @tparam Self The type of the parser.
     * @tparam T The type of the fixed value.
     * @param self The parser instance.
     * @param value The fixed value to replace the result with.
     * @return A new parser that replaces the result of the original parser with the fixed value.
     */
    template <typename Self, typename T>
    constexpr auto value(this Self&& self, T&& value)
    {
        return self.map([v = (T&&) value](auto&&) { return v; });
    }

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




