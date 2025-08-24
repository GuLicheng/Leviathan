/*
    We define some helper classes for sequencing parsers.
*/

#include "error.hpp"
#include <utility>
#include <type_traits>

#pragma once

namespace nom
{
    
template <typename First, typename Second>
class Terminated
{
    First first;
    Second second;

public:

    constexpr Terminated(First f, Second s) 
        : first(std::move(f)), second(std::move(s)) 
    { }

    template <typename ParseContext>
    constexpr auto operator()(ParseContext& ctx)
    {
        using R1 = std::invoke_result_t<First, ParseContext&>;

        auto result1 = first(ctx);

        if (!result1)
        {
            return R1(std::unexpect, std::move(result1.error()));
        }

        if (auto result2 = second(ctx); !result2)
        {
            return R1(std::unexpect, std::move(result2.error()));
        }

        return R1(std::in_place, std::move(*result1));
    }
};

template <typename First, typename Second>
class Preceded
{
    First first;
    Second second;

public:

    constexpr Preceded(First f, Second s) 
        : first(std::move(f)), second(std::move(s)) 
    { }

    template <typename ParseContext>
    constexpr auto operator()(ParseContext& ctx)
    {
        using R2 = std::invoke_result_t<Second, ParseContext&>;

        if (auto result1 = first(ctx); !result1)
        {
            return R2(std::unexpect, std::move(result1.error()));
        }

        auto result2 = second(ctx);

        if (!result2)
        {
            return R2(std::unexpect, std::move(result2.error()));
        }

        return R2(std::in_place, std::move(*result2));
    }
};

template <typename F1, typename F2>
class And
{
    F1 first;
    F2 second;

public:

    constexpr And(F1 f1, F2 f2) : first(std::move(f1)), second(std::move(f2)) {}

    template <typename ParseContext>
    constexpr auto operator()(ParseContext& ctx)
    {
        using R1 = typename std::invoke_result_t<F1, ParseContext&>::value_type;
        using R2 = typename std::invoke_result_t<F2, ParseContext&>::value_type;
        using R = IResult<std::pair<R1, R2>>;

        auto result1 = first(ctx);

        if (!result1)
        {
            return R(std::unexpect, std::move(result1.error()));
        }

        auto result2 = second(ctx);

        if (!result2)
        {
            return R(std::unexpect, std::move(result2.error()));
        }

        return R(std::in_place, std::move(*result1), std::move(*result2));
    }
};

template <typename F1, typename F2>
class Or
{
    F1 first;
    F2 second;

public:

    constexpr Or(F1 f1, F2 f2) : first(std::move(f1)), second(std::move(f2)) {}

    template <typename ParseContext>
    constexpr auto operator()(ParseContext& ctx)
    {
        using R1 = typename std::invoke_result_t<F1, ParseContext&>;
        using R2 = typename std::invoke_result_t<F2, ParseContext&>;
        static_assert(std::is_same_v<R1, R2>);

        // Clone the context, for streaming parsers, 
        // may comsume the context during parsing so 
        // we need to restore it if the first parser fails.
        auto clone = ctx; 

        if (auto result1 = first(clone); result1)
        {
            ctx = std::move(clone); // Restore the context.
            return R1(std::in_place, std::move(*result1));
        }
        else
        {
            auto result2 = second(ctx);
            return result2 ? R1(std::in_place, std::move(*result2)) 
                           : R1(std::unexpect, std::move(result2.error()));
        }
    }
};

template <bool AtLeastOne>
struct MultiSpace
{
    // Recognizes zero or more spaces, tabs, carriage returns and line feeds.
    static constexpr bool check(char c) 
    { 
        return c == ' ' || c == '\t' || c == '\n' || c == '\r'; 
    }

    template <typename ParseContext>
    static constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {   
        auto first = ctx.begin();
        auto last = ctx.end();

        for (; first != last && check(*first); ++first);

        std::string_view result = { ctx.begin(), first };

        if constexpr (AtLeastOne)
        {
            if (result.empty())
            {
                return IResult<std::string_view>(
                    std::unexpect, 
                    "Expected at least one space character",
                    ErrorKind::MultiSpace
                );
            }
        }
        
        // Complete version: will return the whole input if no 
        // terminating token is found (a non space character).
        ctx.remove_prefix(result.size());
        return IResult<std::string_view>(std::in_place, result);
    }

};

// Wrapping structure for the [alt()] combinator implementation
// Tests a list of parsers one by one until one succeeds.
template <typename... Fns>
class Choice
{
    std::tuple<Fns...> fns;
    
    static constexpr size_t N = sizeof...(Fns);

    template <size_t Idx, typename R, typename ParseContext, typename FunctionTuple>
    static auto try_parse_one_by_one(ParseContext& ctx, FunctionTuple&& funcs)
    {
        auto clone = ctx;

        if constexpr (Idx == N - 1)
        {
            if (auto result = std::invoke(std::get<Idx>((FunctionTuple&&)funcs), clone); result)
            {
                ctx = std::move(clone);
                return R(std::in_place, std::move(*result));
            }
            else
            {
                return R(std::unexpect, "All parsers in choice failed", ErrorKind::Alt);
            }
        }
        else
        {
            if (auto result = std::invoke(std::get<Idx>((FunctionTuple&&)funcs), clone); result)
            {
                ctx = std::move(clone);
                return R(std::in_place, std::move(*result));
            }
            else
            {
                return try_parse_one_by_one<Idx + 1, R>(ctx, (FunctionTuple&&)funcs);
            }
        }
    }

public:

    constexpr Choice(Fns... fs) : fns(std::move(fs)...) { }

    template <typename ParseContext>
    constexpr auto operator()(ParseContext& ctx)
    {
        using R1 = std::invoke_result_t<std::tuple_element_t<0, std::tuple<Fns...>>, ParseContext&>;
        static_assert((std::is_same_v<R1, std::invoke_result_t<Fns, ParseContext&>> && ...));
        return try_parse_one_by_one<0, R1>(ctx, fns);
    }
};




















} // namespace nom

