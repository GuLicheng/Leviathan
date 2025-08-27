/*
    We define some helper classes for sequencing parsers.
*/

#include "error.hpp"
#include <utility>
#include <type_traits>
#include <functional>
#include <algorithm>

#pragma once

namespace nom
{

/**
 * @brief We use this binder to store the parser function and its arguments (other parsers or predicates).
 * Each parser function must have the signature like:
 * 
 *  template <typename FunctionTuple, typename ParseContext>
 *  static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx) { ... }
 * 
 *  The first argument is a tuple of all the callables (parsers or predicates).
 *  The second argument is the parsing context (usually a string_view&).
 */
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

// FIXME: this is not used yet.
#if 0
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
#endif

class Tag
{
    std::string_view value;

public:

    constexpr Tag(std::string_view v) : value(v) { }

    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx) const
    {
        if (ctx.starts_with(value))
        {
            ctx.remove_prefix(value.size());
            return value;
        }
        else
        {
            return IResult<std::string_view>(
                std::unexpect, 
                std::format("Tag parser failed: expected '{}', but got {}.", value, ctx),
                ErrorKind::Tag
            );
        }
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

// Wrapping structure for the permutation combinator implementation
// Applies a list of parsers in any order.
// Permutation will succeed if all of the child parsers succeeded. 
// It takes as argument a tuple of parsers, and returns a tuple of the parser results.
struct Permutation;

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
struct Escaped
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr IResult<std::string_view> operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        auto [normal, control_char, escapable] = (FunctionTuple&&)fns;

        using R1 = std::invoke_result_t<std::decay_t<decltype(normal)>, ParseContext&>;
        static_assert(std::is_same_v<R1, IResult<std::string_view>>);

        using R2 = std::invoke_result_t<std::decay_t<decltype(escapable)>, ParseContext&>;
        static_assert(std::is_same_v<R2, IResult<std::string_view>>);

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

template <typename Prediction>
struct ConditionalLoop0
{
    Prediction pred;

    constexpr ConditionalLoop0(Prediction p) : pred(std::move(p)) { }

    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {
        auto first = ctx.begin(), last = ctx.end();

        for (; first != last && std::invoke(pred, *first); ++first);

        std::string_view result = { ctx.begin(), first };
        ctx.remove_prefix(result.size());
        return IResult<std::string_view>(std::in_place, result);
    }
};

template <typename Prediction>
ConditionalLoop0(Prediction&&) -> ConditionalLoop0<std::decay_t<Prediction>>;

template <typename Prediction>
struct ConditionalLoop1 : ConditionalLoop0<Prediction>
{
    using base = ConditionalLoop0<Prediction>;

    ErrorKind kind;

    constexpr ConditionalLoop1(Prediction p, ErrorKind k) : base(std::move(p)), kind(k) { }

    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {
        auto result = base::operator()(ctx);

        if (!result || result->empty())
        {
            return IResult<std::string_view>(std::unexpect, std::string(ctx), kind);
        }

        return result;
    }
};

template <typename Prediction>
ConditionalLoop1(Prediction&&, ErrorKind) -> ConditionalLoop1<std::decay_t<Prediction>>;

struct MultiSpace
{
    // Recognizes zero or more spaces, tabs, carriage returns and line feeds.
    template <typename CharT>
    static constexpr bool operator()(CharT c) 
    { 
        return c == CharT(' ') 
            || c == CharT('\t') 
            || c == CharT('\r') 
            || c == CharT('\n');
    }
};

struct Space
{
    template <typename CharT>
    static constexpr bool operator()(CharT c) 
    { 
        return c == CharT(' ') || c == CharT('\t');
    }
};

template <bool AtLeastOne>
struct SeparatedList
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        // using R1 = std::invoke_result_t<std::tuple_element_t<0, std::decay_t<FunctionTuple>>, ParseContext&>;
        using R2 = std::invoke_result_t<std::tuple_element_t<1, std::decay_t<FunctionTuple>>, ParseContext&>;
        using R = IResult<std::vector<typename R2::value_type>>;

        auto [sep_parser, item_parser] = (FunctionTuple&&)fns;
        std::vector<typename R2::value_type> items;

        // First try to parse the first item
        auto item_result = item_parser(ctx);

        if (!item_result)
        {
            if constexpr (AtLeastOne)
            {
                return R(std::unexpect, std::move(item_result.error()));
            }
            else
            {
                return R(std::in_place, std::move(items)); // empty list
            }
        }
        
        items.emplace_back(std::move(*item_result));

        while (true)
        {
            auto clone = ctx;

            if (auto sep_result = sep_parser(clone); !sep_result)
            {
                break;
            }

            if (auto item_result = item_parser(clone); !item_result)
            {
                break;
            }
            else
            {
                ctx = std::move(clone);
                items.emplace_back(std::move(*item_result));
            }
        }

        return R(std::in_place, std::move(items));
    }
};

template <typename Prediction>
struct CheckFirstCharacter
{
    Prediction pred;
    ErrorKind kind;

    constexpr CheckFirstCharacter(Prediction p, ErrorKind k) : pred(std::move(p)), kind(k) { }

    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {
        if (ctx.empty())
        {
            return IResult<std::string_view>(std::unexpect, std::string(ctx), kind);
        }

        if (!std::invoke(pred, ctx[0]))
        {
            return IResult<std::string_view>(std::unexpect, std::string(ctx), kind);
        }

        std::string_view retval = ctx.substr(0, 1);
        ctx.remove_prefix(1);
        return IResult<std::string_view>(std::in_place, retval);
    }
};

template <typename Prediction>
CheckFirstCharacter(Prediction&&, ErrorKind) -> CheckFirstCharacter<std::decay_t<Prediction>>;

struct Many0
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        using R1 = std::invoke_result_t<std::tuple_element_t<0, std::decay_t<FunctionTuple>>, ParseContext&>;
        using R = IResult<std::vector<typename R1::value_type>>;

        auto [parser] = (FunctionTuple&&)fns;
        std::vector<typename R1::value_type> items;

        while (true)
        {
            if (ctx.empty())
            {
                break;
            }

            auto clone = ctx;

            if (auto result = parser(clone); !result)
            {
                break;
            }
            else
            {
                ctx = std::move(clone);
                items.emplace_back(std::move(*result));
            }
        }

        return R(std::in_place, std::move(items));
    }    
};

struct Many1
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        auto result = Many0::operator()(std::forward<FunctionTuple>(fns), ctx);
        using R = decltype(result);

        // Many0 always succeed, so we need to check the result here.
        if (result->empty())
        {
            return R(std::unexpect, std::string(ctx), ErrorKind::Many1);
        }

        return result;
    }    
};

struct Count
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        using R1 = std::invoke_result_t<std::tuple_element_t<0, std::decay_t<FunctionTuple>>, ParseContext&>;
        using R = IResult<std::vector<typename R1::value_type>>;

        auto [parser, cnt] = (FunctionTuple&&)fns;
        std::vector<typename R1::value_type> items;

        for (size_t i = 0; i < cnt; ++i)
        {
            auto result = parser(ctx);

            if (!result)
            {
                return R(std::unexpect, std::move(result.error()));
            }

            items.emplace_back(std::move(*result));
        }

        return R(std::in_place, std::move(items));
    }    
};

// Runs the embedded parser repeatedly, filling the given slice with results.
// This parser fails if the input runs out before the given slice is full.
// Since we use an output iterator, the output container must be pre-sized.
// So we cannot check whether the given slice is full or not, we add
// another argument `sentinel`, which used for indicate whether the
// parse should be stopped.
struct Fill
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        auto [parser, iterator, sentinel] = (FunctionTuple&&)fns;

        using R = IResult<std::decay_t<decltype(iterator)>>;

        for (; iterator != sentinel; )
        {   
            auto result = parser(ctx);

            if (!result)
            {
                return R(std::unexpect, std::move(result.error()));
            }

            *iterator++ = std::move(*result);
        }

        return R(std::in_place, std::move(iterator));
    }  
};

struct ManyFolder
{
    template <typename FunctionTuple, typename ParseContext>
    static constexpr auto operator()(FunctionTuple&& fns, ParseContext& ctx)
    {
        auto [many_parser, init, fn] = (FunctionTuple&&)fns;

        auto result = many_parser(ctx);

        using R = decltype(result);

        if (!result)
        {
            return R(std::unexpect, std::move(result.error())); 
        }

        return R(std::in_place, std::ranges::fold_left(std::move(*result), std::move(init), std::move(fn)));
    }
};


#if 0
template <typename AllowEmpty>
struct RequireNonEmpty 
{
    AllowEmpty allow_empty;
    ErrorKind kind;

    constexpr RequireNonEmpty(AllowEmpty ae, ErrorKind k) : allow_empty(std::move(ae)), kind(k) { }

    template <typename ParseContext>
    constexpr auto operator()(ParseContext& ctx)
    {
        auto result = allow_empty(ctx);

        using R = decltype(result);

        if (!result || std::ranges::empty(*result))
        {
            return R(std::unexpect, std::string(ctx), kind);
        }

        return result;
    }    
};

template <typename AllowEmpty>
RequireNonEmpty(AllowEmpty&&, ErrorKind) -> RequireNonEmpty<std::decay_t<AllowEmpty>>;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
struct LineEnding
{
    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {
        if (ctx.starts_with('\n'))
        {
            std::string_view result = ctx.substr(0, 1);
            ctx.remove_prefix(1);
            return IResult<std::string_view>(std::in_place, result);
        }
        else if (ctx.starts_with("\r\n"))
        {
            std::string_view result = ctx.substr(0, 2);
            ctx.remove_prefix(2);
            return IResult<std::string_view>(std::in_place, result);
        }
        else
        {
            return IResult<std::string_view>(std::unexpect, std::string(ctx), ErrorKind::CrLf);
        }
    }
};

struct NotLineEnding
{
    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {
        const auto idx = ctx.find_first_of("\r\n");
        
        if (idx == ctx.npos)
        {
            std::string_view result = ctx;
            ctx.remove_prefix(ctx.size());
            return IResult<std::string_view>(std::in_place, result);
        }

        if (ctx[idx] == '\n')
        {
            std::string_view result = ctx.substr(0, idx);
            ctx.remove_prefix(idx);
            return IResult<std::string_view>(std::in_place, result);
        }
        else if (ctx.substr(idx, 2).starts_with("\r\n"))
        {
            std::string_view result = ctx.substr(0, idx);
            ctx.remove_prefix(idx);
            return IResult<std::string_view>(std::in_place, result);
        }
        else
        {
            return IResult<std::string_view>(std::unexpect, std::string(ctx), ErrorKind::Tag);
        }
    }
};

struct Ctrl
{
    template <typename ParseContext>
    constexpr IResult<std::string_view> operator()(ParseContext& ctx)
    {
        if (ctx.starts_with("\r\n"))
        {
            std::string_view result = ctx.substr(0, 2);
            ctx.remove_prefix(2);
            return IResult<std::string_view>(std::in_place, result);
        }
        else
        {
            return IResult<std::string_view>(std::unexpect, std::string(ctx), ErrorKind::CrLf);
        }
    }
};


} // namespace nom

