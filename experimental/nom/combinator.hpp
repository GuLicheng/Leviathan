#pragma once

#include "error.hpp"

namespace nom::combinator
{
    
inline constexpr struct 
{
    template <typename T, typename F>
    static constexpr auto operator()(T&& v, F&& f)
    {
        auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
        {
            using R = IResult<std::decay_t<T>>;

            auto [value, parser] = (FunctionTuple&&)fns;
            auto result = parser(ctx);

            return result ? R(std::in_place, std::move(value)) 
                          : R(std::unexpect, std::move(result.error()));
        };

        return make_parser_binder(fn, (T&&)v, (F&&)f);
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

inline constexpr struct
{
    template <typename F1>
    static constexpr auto operator()(F1& f1)
    {   
        auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
        {
            using R1 = std::invoke_result_t<std::tuple_element_t<0, std::decay_t<FunctionTuple>>, ParseContext&>;
            using Optional = std::optional<typename R1::value_type>;
            using R = IResult<Optional>;

            auto [parser] = (FunctionTuple&&)fns;
            auto result = parser(ctx);
            
            return result ? R(std::in_place, std::in_place, std::move(*result))
                          : R(std::in_place, std::nullopt); // always succeed
        };
        return make_parser_binder(fn, (F1&&)f1);
    }
} opt;






} // namespace nom

