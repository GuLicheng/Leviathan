#pragma once

#include "error.hpp"

namespace nom::combinator
{
    
template <typename T, typename F>
class Value
{
    T value;
    F parser;

public:

    constexpr Value(T v, F f) : value(std::move(v)), parser(std::move(f)) { }

    template <typename ParseContext>
    constexpr IResult<T> operator()(ParseContext& ctx)
    {
        auto result = parser(ctx); 
        
        if (result)
        {
            return std::move(value);
        }
        else
        {
            return IResult<T>(std::unexpect, std::move(result.error()));
        }
    }
};

inline constexpr struct 
{
    template <typename T, typename F>
    static constexpr auto operator()(T v, F f)
    {
        return Value<T, F>(std::move(v), std::move(f));
    }
} value;

// Tries to apply its parser without consuming the input.
inline constexpr struct 
{
    template <typename F1>
    static constexpr auto operator()(F1&& f1)
    {
        auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
        {
            auto [f] = (FunctionTuple&&)fns;
            auto clone = ctx; // copy
            auto result = f(clone);
            return result;
        };

        return make_parser_binder(fn, (F1&&)f1);
    }
} peek;

inline constexpr struct
{
    template <typename ParserFunction, typename MapFunction>
    static constexpr auto operator()(ParserFunction&& pf, MapFunction&& mf)
    {
        auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
        {
            using R1 = std::invoke_result_t<std::decay_t<ParserFunction>, ParseContext&>;
            using R = IResult<std::invoke_result_t<std::decay_t<MapFunction>, typename R1::value_type>>;

            auto [parser, mapper] = (FunctionTuple&&)fns;
            auto result = parser(ctx);

            return result ? R(std::in_place, mapper(std::move(*result))) 
                          : R(std::unexpect, std::move(result.error()));
        };

        return make_parser_binder(fn, (ParserFunction&&)pf, (MapFunction&&)mf); 
    }
} map;

} // namespace nom

