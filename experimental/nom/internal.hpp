/*
    We define some helper classes for sequencing parsers.
*/

#include "error.hpp"
#include <utility>
#include <type_traits>

#pragma once

namespace nom
{
    
template <typename ParseFunction, typename... Callables>
class ParserBinder 
{
    ParseFunction pf;
    std::tuple<Callables...> callables;

public:

    constexpr ParserBinder(ParseFunction pf, Callables... callables) : pf(pf), callables(std::move(callables)...) 
    { }

    template <typename Self, typename ParseContext>
    constexpr auto operator()(this Self&& self, ParseContext& ctx)
    {
        return std::invoke(std::forward_like<Self>(self.pf), std::forward_like<Self>(self.callables), ctx);
    }
};

inline constexpr struct
{
    template <typename ParseFunction, typename... Callables>
    static constexpr ParserBinder<std::decay_t<ParseFunction>, std::decay_t<Callables>...> 
    operator()(ParseFunction&& pf, Callables&&... callables)
    {
        return ParserBinder<std::decay_t<ParseFunction>, std::decay_t<Callables>...>(
            (ParseFunction&&)pf, (Callables&&)callables ...
        );
    }
} make_parser_binder;

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
    template <typename CharT>
    static constexpr bool check(CharT c) 
    { 
        return c == CharT(' ') 
            || c == CharT('\t') 
            || c == CharT('\r') 
            || c == CharT('\n');
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

template <bool AtLeastOne>
struct Dight
{
    template <typename ParseContext>
    static constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {
        auto first = ctx.begin(), last = ctx.end();

        for (; first != last && std::isdigit(*first); ++first);

        std::string_view result = { ctx.begin(), first };

        if constexpr (AtLeastOne)
        {
            if (result.empty())
            {
                return IResult<std::string_view>(
                    std::unexpect, 
                    "Expected at least one digit",
                    ErrorKind::Digit
                );
            }
        }

        ctx.remove_prefix(result.size());
        return IResult<std::string_view>(std::in_place, result);
    }
};

template <bool AtLeastOne>
struct TakeTill
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        auto [prediction] = (FunctionTuple&&)fns;
        auto first = ctx.begin(), last = ctx.end();

        for (; first != last && !std::invoke(prediction, *first); ++first);

        std::string_view result = { ctx.begin(), first };

        if constexpr (AtLeastOne)
        {
            if (result.empty())
            {
                return IResult<std::string_view>(
                    std::unexpect, 
                    "Expected at least one character",
                    ErrorKind::TakeTill1
                );
            }
        }

        ctx.remove_prefix(result.size());
        return IResult<std::string_view>(std::in_place, result);
    }
};

// Wrapping structure for the [alt()] combinator implementation
// Tests a list of parsers one by one until one succeeds.
struct Choice
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        using DecayTuple = std::decay_t<FunctionTuple>;
        using R1 = std::invoke_result_t<std::tuple_element_t<0, DecayTuple>, ParseContext&>;
        using R = IResult<typename R1::value_type>;
        return try_parse_one_by_one<0, R>(ctx, (FunctionTuple&&)fns);
    }

    template <size_t Idx, typename R, typename ParseContext, typename FunctionTuple>
    static auto try_parse_one_by_one(ParseContext& ctx, FunctionTuple&& funcs)
    {
        constexpr auto N = std::tuple_size_v<std::remove_reference_t<FunctionTuple>>;

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
};

struct OneOf
{
    std::string_view chars;

    constexpr OneOf(std::string_view s) : chars(s) { }

    template <typename ParseContext>
    constexpr IResult<char> operator()(ParseContext& ctx)
    {
        const auto idx = ctx.find_first_of(chars);

        if (idx == ctx.npos)
        {
            return IResult<char>(
                std::unexpect, 
                std::format("Expected one of the characters: '{}'", chars),
                ErrorKind::OneOf
            );
        }

        auto ch = ctx[idx];
        ctx.remove_prefix(idx + 1);
        return IResult<char>(std::in_place, ch);
    }
};

// Matches a byte string with escaped characters.
// The first argument matches the normal characters (it must not accept the control character)
// The second argument is the control character (like \ in most languages)
// The third argument matches the escaped characters
template <typename Normal, typename ControlChar, typename Escapable>
class Escaped
{
    Normal normal;
    ControlChar control_char;
    Escapable escapable;

public:

    constexpr Escaped(Normal n, ControlChar c, Escapable e) 
        : normal(std::move(n)), control_char(std::move(c)), escapable(std::move(e)) 
    { }

    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {   
        using R1 = std::invoke_result_t<Normal, ParseContext&>;
        static_assert(std::is_same_v<R1, IResult<std::string_view>>);

        using R2 = std::invoke_result_t<Escapable, ParseContext&>;
        static_assert(std::is_same_v<R2, IResult<char>>);

        auto start = ctx.begin();

        while (true)
        {
            auto result = normal(ctx);

            if (!result || ctx.empty() || ctx[0] != control_char)
            {
                std::string_view result = { start, ctx.begin() };
                return IResult<std::string_view>(std::in_place, result);
            }

            // Skip the control character
            ctx.remove_prefix(1);

            // Now parse the escaped character
            auto esc_result = escapable(ctx);

            if (!esc_result)
            {
                std::string_view result = { start, ctx.begin() };
                return IResult<std::string_view>(std::in_place, result);
            }
        }
    }

};  









} // namespace nom

