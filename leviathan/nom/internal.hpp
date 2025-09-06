/*
    We define some helper classes for sequencing parsers.
*/
#pragma once

#include "error.hpp"
#include <leviathan/extc++/concepts.hpp>
#include <utility>
#include <type_traits>
#include <functional>
#include <algorithm>

namespace nom
{

template <typename Parser, typename Context>
concept parser = requires(Parser p, Context ctx)
{
    typename Parser::output_type;
    typename Parser::error_type;
    // { p(ctx) } -> std::same_as<typename Parser::output_type>;
};
    
struct parser_interface 
{
};

namespace detail
{

template <typename Context>
struct tag_parser
{
    using error_type = error<Context, error_kind>;
    using char_type = typename Context::value_type;
    using tag_type = std::basic_string_view<char_type>;
    using output_type = Context;
    using result_type = iresult<Context, output_type, error_type>;

    tag_type tag_value;

    constexpr tag_parser(tag_type t) : tag_value(t) { }

    constexpr result_type operator()(Context ctx) const
    {
        if (ctx.match(tag_value, false))
        {
            auto [left, right] = ctx.split_at(tag_value.size());
            return result_type(rust::in_place, std::move(right), std::move(left));
        }
        else
        {
            return result_type(rust::unexpect, std::move(ctx), error_kind::tag);
        }
    }
};

template <typename Context, typename Prediction, typename ErrorCode = error_kind>
struct conditional_loop0
{
    using char_type = typename Context::value_type;
    using output_type = Context;
    using error_type = error<Context, ErrorCode>;
    using result_type = iresult<Context, output_type, error_type>;

    Prediction pred;

    constexpr conditional_loop0(Prediction p) : pred(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto first = ctx.begin(), last = ctx.end();

        for (; first != last && std::invoke(pred, *first); ++first);

        auto [left, right] = ctx.split_at(std::distance(ctx.begin(), first));
        return result_type(rust::in_place, std::move(right), std::move(left));
    }
};

template <typename Context, typename Prediction, typename ErrorCode = error_kind>
struct conditional_loop1 : conditional_loop0<Context, Prediction, ErrorCode>
{
    using base = conditional_loop0<Context, Prediction, ErrorCode>;
    using typename base::result_type;

    ErrorCode code;

    constexpr conditional_loop1(Prediction p, ErrorCode c) : base(std::move(p)), code(c) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result = base::operator()(ctx);

        if (!result || result->second.empty())
        {
            return result_type(rust::unexpect, std::move(ctx), code);
        }

        return result;
    }
};

template <typename Context>
struct take_parser
{
    using char_type = typename Context::value_type;
    using output_type = Context;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;
    using tag_type = std::basic_string_view<char_type>;

    size_t count;

    constexpr take_parser(size_t c) : count(c) { }

    constexpr result_type operator()(Context ctx) const
    {
        if (ctx.size() < count)
        {
            return result_type(rust::unexpect, std::move(ctx), error_kind::eof);
        }

        auto [left, right] = ctx.split_at(count);
        return result_type(rust::in_place, std::move(right), std::move(left));
    }
};

template <typename Context, typename Normal, typename ControlChar, typename Escapable>
struct escaped_parser
{
    using char_type = typename Context::value_type;
    using output_type = Context;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    Normal normal;
    ControlChar control_char;
    Escapable escapable;

    constexpr escaped_parser(Normal n, ControlChar c, Escapable e) 
        : normal(std::move(n)), control_char(std::move(c)), escapable(std::move(e)) 
    { }

    constexpr result_type operator()(Context ctx) 
    {
        auto clone = ctx;

        while (1)
        {
            auto result = normal(ctx);

            if (!result)
            {
                // output_type slice = { start, ctx.begin() };
                // return result_type(rust::in_place, std::move(ctx), slice);
                auto [left, _] = clone.split_at(std::distance(clone.begin(), ctx.begin()));
                return result_type(rust::in_place, std::move(ctx), std::move(left));
            }

            ctx = std::move(result->first);

            if (ctx.empty() || ctx[0] != control_char)
            {
                auto [left, _] = clone.split_at(std::distance(clone.begin(), ctx.begin()));
                return result_type(rust::in_place, std::move(ctx), std::move(left));
            }

            // Skip the control character
            ctx.advance(1);

            auto esc_result = escapable(ctx);

            if (!esc_result)
            {
                auto [left, _] = clone.split_at(std::distance(clone.begin(), ctx.begin()));
                return result_type(rust::in_place, std::move(ctx), std::move(left));
            }

            ctx = std::move(esc_result->first);
        }
    }
};

template <typename Context, typename Prediction, typename ErrorCode = error_kind>
struct check_first_character
{
    using char_type = typename Context::value_type;
    using output_type = Context;
    using error_type = error<Context, ErrorCode>;
    using result_type = iresult<Context, output_type, error_type>;

    Prediction pred;
    ErrorCode code;

    constexpr check_first_character(Prediction p, ErrorCode c) : pred(std::move(p)), code(c) { }

    constexpr result_type operator()(Context ctx)
    {
        if (ctx.empty())
        {
            return result_type(std::unexpect, std::move(ctx), code);
        }

        if (!std::invoke(pred, ctx[0]))
        {
            return result_type(std::unexpect, std::move(ctx), code);
        }

        auto [left, right] = ctx.split_at(1);
        return result_type(rust::in_place, std::move(right), std::move(left));
    }
};

template <typename CharT>
struct tag_fn
{
    std::basic_string_view<CharT> tag_value;

    constexpr tag_fn(std::basic_string_view<CharT> t) : tag_value(t) { }
    constexpr tag_fn(const CharT* t) : tag_value(t) { }

    template <typename Context>
    constexpr auto operator()(Context ctx)
    {
        return tag_parser<Context>(tag_value)(std::move(ctx));
    }
};

template <typename CharT>
struct is_a_fn
{
    std::basic_string_view<CharT> pattern;

    constexpr is_a_fn(std::basic_string_view<CharT> t) : pattern(t) { }
    constexpr is_a_fn(const CharT* t) : pattern(t) { }

    template <typename Context>
    constexpr auto operator()(Context ctx)
    {
        auto searcher = [this](CharT ch) { return pattern.contains(ch); };
        return conditional_loop1<context, decltype(searcher)>(searcher, error_kind::is_a)(std::move(ctx));
    }
};

template <typename CharT>
struct is_not_fn
{
    std::basic_string_view<CharT> pattern;

    constexpr is_not_fn(std::basic_string_view<CharT> t) : pattern(t) { }
    constexpr is_not_fn(const CharT* t) : pattern(t) { }

    template <typename Context>
    constexpr auto operator()(Context ctx)
    {
        auto searcher = [this](CharT ch) { return !pattern.contains(ch); };
        return conditional_loop1<context, decltype(searcher)>(searcher, error_kind::is_not)(std::move(ctx));
    }
};

struct ismultispace
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

struct isspace
{
    template <typename CharT>
    static constexpr bool operator()(CharT c) 
    { 
        return c == CharT(' ') || c == CharT('\t');
    }
};

template <typename CharT>
struct one_of_fn
{
    std::basic_string_view<CharT> pattern;

    constexpr one_of_fn(std::basic_string_view<CharT> t) : pattern(t) { }
    constexpr one_of_fn(const CharT* t) : pattern(t) { }

    template <typename Context>
    constexpr auto operator()(Context ctx)
    {
        auto pred = [this](CharT c) { return pattern.contains(c); };
        return check_first_character<Context, decltype(pred), error_kind>(std::move(pred), error_kind::one_of)(std::move(ctx));
    }
};

template <typename CharT>
struct none_of_fn
{
    std::basic_string_view<CharT> pattern;

    constexpr none_of_fn(std::basic_string_view<CharT> t) : pattern(t) { }
    constexpr none_of_fn(const CharT* t) : pattern(t) { }

    template <typename Context>
    constexpr auto operator()(Context ctx)
    {
        auto pred = [this](CharT c) { return !pattern.contains(c); };
        return check_first_character<Context, decltype(pred), error_kind>(std::move(pred), error_kind::none_of)(std::move(ctx));
    }
};

template <typename Context>
struct crlf_fn
{
    using char_type = typename Context::value_type;
    using output_type = Context;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    static constexpr result_type operator()(Context ctx)
    {
        if (ctx.match("\r\n", false))
        {
            auto [left, right] = ctx.split_at(2);
            return result_type(rust::in_place, std::move(right), std::move(left));
        }
        else
        {
            return result_type(rust::unexpect, std::move(ctx), error_kind::crlf);
        }
    }
};

template <typename Context>
struct line_ending_fn
{
    using char_type = typename Context::value_type;
    using output_type = Context;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    static constexpr result_type operator()(Context ctx)
    {
        if (ctx.match('\n', false))
        {
            auto [left, right] = ctx.split_at(1);
            return result_type(rust::in_place, std::move(right), std::move(left));
        }
        else if (ctx.match("\r\n", false))
        {
            auto [left, right] = ctx.split_at(2);
            return result_type(rust::in_place, std::move(right), std::move(left));
        }
        else
        {
            return result_type(rust::unexpect, std::move(ctx), error_kind::crlf);
        }
    }
};

template <typename Context>
struct not_line_ending_fn
{
    using char_type = typename Context::value_type;
    using output_type = Context;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    static constexpr result_type operator()(Context ctx)
    {
        const auto idx = ctx.find_first_of("\r\n");
        
        if (idx == ctx.npos)
        {
            auto [left, right] = ctx.split_at(ctx.size());
            return result_type(rust::in_place, std::move(right), std::move(left));
        }

        if (ctx[idx] == '\n')
        {
            auto [left, right] = ctx.split_at(idx);
            return result_type(rust::in_place, std::move(right), std::move(left));
        }
        else if (ctx.peek(idx + 1) == '\n')
        {
            auto [left, right] = ctx.split_at(idx);
            return result_type(rust::in_place, std::move(right), std::move(left));
        }
        else
        {
            return result_type(rust::unexpect, std::move(ctx), error_kind::tag);
        }
    }
};

template <typename T1, typename... Ts>
inline constexpr bool all_same = (std::is_same_v<T1, Ts> && ... );

template <typename Context, typename Parser1, typename Parser2>
struct pair_fn
{
    using result_type1 = std::invoke_result_t<Parser1, Context>;  // iresult<Context, T1, E>
    using result_type2 = std::invoke_result_t<Parser2, Context>;  // iresult<Context, T2, E>

    static_assert(all_same<typename result_type1::error_type, typename result_type2::error_type>);
    static_assert(all_same<Context, typename result_type1::input_type, typename result_type2::input_type>);

    using output_type = std::pair<
        typename result_type1::output_type, 
        typename result_type2::output_type
    >;

    using result_type = iresult<Context, output_type, typename result_type1::error_type>;
    
    Parser1 first;
    Parser2 second;

    constexpr pair_fn(Parser1 f1, Parser2 f2) : first(std::move(f1)), second(std::move(f2)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result1 = first(ctx);

        if (!result1)
        {
            return result_type(rust::unexpect, std::move(ctx), result1.error().code);
        }

        ctx = std::move(result1->first);
        auto result2 = second(ctx);

        if (!result2)
        {
            return result_type(rust::unexpect, std::move(ctx), result2.error().code);
        }

        return result_type(
            rust::in_place, 
            std::move(result2->first), 
            output_type(std::move(result1->second), std::move(result2->second))
        );
    }
};

template <typename Context, typename Parser1, typename Parser2>
struct preceded_fn
{
    using result_type1 = std::invoke_result_t<Parser1, Context>;  // iresult<Context, T1, E>
    using result_type2 = std::invoke_result_t<Parser2, Context>;  // iresult<Context, T2, E>
    using result_type = result_type2;

    static_assert(all_same<typename result_type1::error_type, typename result_type2::error_type>);
    static_assert(all_same<Context, typename result_type1::input_type, typename result_type2::input_type>);

    static_assert(all_same<Context, typename result_type1::input_type, typename result_type2::input_type>);

    Parser1 first;
    Parser2 second;

    constexpr preceded_fn(Parser1 f1, Parser2 f2) : first(std::move(f1)), second(std::move(f2)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result1 = first(ctx);

        if (!result1)
        {
            return result_type(rust::unexpect, std::move(ctx), result1.error().code);
        }

        ctx = std::move(result1->first);
        auto result2 = second(ctx);

        if (!result2)
        {
            return result_type(rust::unexpect, std::move(ctx), result2.error().code);
        }

        return result_type(rust::in_place, std::move(result2->first), std::move(result2->second));
    }
};

template <typename Context, typename Parser1, typename Parser2>
struct terminated_fn
{
    using result_type1 = std::invoke_result_t<Parser1, Context>;  // iresult<Context, T1, E>
    using result_type2 = std::invoke_result_t<Parser2, Context>;  // iresult<Context, T2, E>
    using result_type = result_type1;

    static_assert(all_same<typename result_type1::error_type, typename result_type2::error_type>);
    static_assert(all_same<Context, typename result_type1::input_type, typename result_type2::input_type>);

    Parser1 first;
    Parser2 second;

    constexpr terminated_fn(Parser1 f1, Parser2 f2) : first(std::move(f1)), second(std::move(f2)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result1 = first(ctx);

        if (!result1)
        {
            return result_type(rust::unexpect, std::move(ctx), result1.error().code);
        }

        ctx = std::move(result1->first);
        auto result2 = second(ctx);

        if (!result2)
        {
            return result_type(rust::unexpect, std::move(ctx), result2.error().code);
        }

        return result_type(rust::in_place, std::move(result2->first), std::move(result1->second));
    }
};

template <typename Context, typename... Parsers>
struct choice
{
    using result_type1 = std::invoke_result_t<std::tuple_element_t<0, std::tuple<Parsers...>>, Context>;
    using output_type = typename result_type1::output_type;
    using result_type = iresult<Context, output_type, error<Context, error_kind>>;

    static_assert((std::is_same_v<typename std::invoke_result_t<Parsers, Context>::input_type, Context> && ...));
    static_assert((std::is_same_v<typename std::invoke_result_t<Parsers, Context>::output_type, output_type> && ...));

    static constexpr auto N = sizeof...(Parsers);

    std::tuple<Parsers...> parsers;

    constexpr choice(Parsers... ps) : parsers(std::move(ps)...) { }

    template <std::size_t Idx>
    constexpr result_type try_parsers(Context ctx)
    {
        if constexpr (Idx == N - 1)
        {
            auto result = std::get<Idx>(parsers)(ctx);
            return result ? result_type(rust::in_place, std::move(result->first), std::move(result->second)) 
                          : result_type(rust::unexpect, std::move(ctx), error_kind::alt);
        }
        else
        {
            auto result = std::get<Idx>(parsers)(ctx);
            return result ? result_type(rust::in_place, std::move(result->first), std::move(result->second)) 
                          : try_parsers<Idx + 1>(std::move(ctx));
        }
    }

    constexpr result_type operator()(Context ctx)
    {
        return try_parsers<0>(std::move(ctx));
    }
};

template <typename Context, typename Value, typename Parser>
struct value_parser
{
    using char_type = typename Context::value_type;
    using output_type = Value;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    Value value;
    Parser parser;

    constexpr value_parser(Value v, Parser p) : value(std::move(v)), parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx) 
    {
        auto result = parser(ctx);
        return result ? result_type(rust::in_place, std::move(result->first), value)
                      : result_type(rust::unexpect, std::move(ctx), result.error().code);
    }
};

template <typename Context>
struct eof_parser
{
    using char_type = typename Context::value_type;
    using output_type = Context;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    static constexpr result_type operator()(Context ctx) 
    {
        if (ctx.empty())
        {
            auto clone = ctx;
            return result_type(rust::in_place, std::move(ctx), std::move(clone));
        }
        else
        {
            return result_type(rust::unexpect, std::move(ctx), error_kind::eof);
        }
    }
};

template <typename Context, typename Output>
struct fail_parser
{
    using char_type = typename Context::value_type;
    using output_type = Output;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    static constexpr result_type operator()(Context ctx) 
    {
        return result_type(rust::unexpect, std::move(ctx), error_kind::fail);
    }
};

template <typename T>
struct fail_fn
{
    template <typename Context>
    constexpr auto operator()(Context ctx)
    {
        return fail_parser<Context, T>()(std::move(ctx));
    }
};

template <typename Context, typename F1, typename M1>
struct map_parser
{
    using result_type1 = std::invoke_result_t<F1, Context>;  // iresult<Context, T1, E>
    using output_type = std::invoke_result_t<M1, typename result_type1::output_type>;
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    F1 parser;
    M1 mapper;

    constexpr map_parser(F1 f1, M1 m1) : parser(std::move(f1)), mapper(std::move(m1)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result = parser(ctx);

        return result ? result_type(rust::in_place, std::move(result->first), mapper(std::move(result->second)))
                      : result_type(rust::unexpect, std::move(ctx), result.error().code);
    }
};

template <typename Context, typename F1, typename F2>
struct map_parser_parser
{
    using result_type1 = std::invoke_result_t<F1, Context>;  // iresult<Context, T1, E>
    using output_type1 = typename result_type1::output_type;
    using input_type1 = typename result_type1::input_type;
    using error_type1 = typename result_type1::error_type;

    using result_type2 = std::invoke_result_t<F2, output_type1>;  // iresult<Context, T2, E>
    using output_type2 = typename result_type2::output_type;
    using error_type2 = typename result_type2::error_type;
    using result_type = iresult<Context, output_type2>;

    static_assert(all_same<typename result_type1::input_type, Context>);
    static_assert(all_same<error_type1, error_type2>);

    F1 parser;
    F2 mapper;

    constexpr map_parser_parser(F1 f1, F2 m1) : parser(std::move(f1)), mapper(std::move(m1)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result1 = parser(ctx);

        if (!result1)
        {
            return result_type(rust::unexpect, std::move(ctx), result1.error().code);
        }

        ctx = std::move(result1->first);
        auto o1 = std::move(result1->second);
        auto result2 = mapper(o1);

        return result2 ? result_type(rust::in_place, std::move(std::move(ctx)), std::move(result2->second))
                       : result_type(rust::unexpect, std::move(o1), result2.error().code);
    }
};

template <typename Context, typename Parser>
struct opt_parser
{
    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    using output_type = std::optional<typename result_type1::output_type>;
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    Parser parser;

    constexpr opt_parser(Parser p) : parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result = parser(ctx);

        return result ? result_type(rust::in_place, std::move(result->first), output_type(std::in_place, std::move(result->second)))
                      : result_type(rust::in_place, std::move(ctx), std::nullopt);
    }
};

template <typename Context, typename Parser>
struct peek_parser
{
    using result_type = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    static_assert(std::is_same_v<typename result_type::input_type, Context>);

    Parser parser;

    constexpr peek_parser(Parser p) : parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result = parser(ctx);
        return result ? result_type(rust::in_place, std::move(ctx), std::move(result->second))
                      : result_type(rust::unexpect, std::move(ctx), result.error().code);
    }
};

template <typename Context, typename Parser>
struct cond_parser
{
    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    using output_type = std::optional<typename result_type1::output_type>;
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    bool condition;
    Parser parser;

    constexpr cond_parser(bool b, Parser p) : condition(b), parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        if (condition)
        {
            auto result = parser(ctx);

            return result ? result_type(rust::in_place, std::move(result->first), output_type(std::in_place, std::move(result->second)))
                          : result_type(rust::unexpect, std::move(ctx), result.error().code);
        }
        else
        {
            return result_type(rust::in_place, std::move(ctx), std::nullopt);
        }
    }
};

template <typename Context, typename Parser>
struct recognize_parser
{
    using output_type = Context;
    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    Parser parser;

    constexpr recognize_parser(Parser p) : parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        if (auto result = parser(ctx); result)
        {
            ctx.advance(result->first.size());
            return result_type(rust::in_place, std::move(result->first), std::move(ctx));
        }
        else
        {
            return result_type(rust::unexpect, std::move(result.error().input), result.error().code);
        }
    }
};

template <typename Context>
struct rest_parser
{
    using output_type = Context;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    static constexpr result_type operator()(Context& ctx)
    {
        auto clone = ctx;
        ctx.advance(ctx.size());
        return result_type(rust::in_place, std::move(ctx), std::move(clone));
    }
};

template <typename Context>
struct rest_len_parser
{
    using output_type = size_t;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    static constexpr result_type operator()(Context& ctx)
    {
        const output_type size = ctx.size();
        return result_type(rust::in_place, std::move(ctx), size);
    }
};

template <typename Context, typename Parser>
struct not_parser
{
    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    using output_type = rust::unit;
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    Parser parser;

    constexpr not_parser(Parser p) : parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result = parser(ctx);

        if (result)
        {
            return result_type(rust::unexpect, std::move(ctx), error_kind::not_);
        }
        else
        {
            return result_type(rust::in_place, std::move(ctx), output_type{});
        }
    }
};

template <typename Context, typename T>
struct succeed_parser
{
    using output_type = T;
    using error_type = error<Context, error_kind>;
    using result_type = iresult<Context, output_type, error_type>;

    T val;

    constexpr succeed_parser(T v) : val(std::move(v)) { }

    constexpr result_type operator()(Context ctx)
    {
        return result_type(rust::in_place, std::move(ctx), val);
    }
};

template <typename Context, typename F1, typename P1>
struct verify_parser
{
    using result_type1 = std::invoke_result_t<F1, Context>;  // iresult<Context, T1, E>
    using output_type = typename result_type1::output_type;
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    F1 parser;
    P1 predicate;

    constexpr verify_parser(F1 f1, P1 p1) : parser(std::move(f1)), predicate(std::move(p1)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result = parser(ctx);

        if (!result)
        {
            return result_type(rust::unexpect, std::move(ctx), result.error().code);
        }

        if (!predicate(result->second))
        {
            return result_type(rust::unexpect, std::move(ctx), error_kind::verify);
        }

        return result_type(rust::in_place, std::move(result->first), std::move(result->second));
    }
};

template <typename Context, typename Parser>
struct all_consuming_parser
{
    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    using output_type = typename result_type1::output_type;
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);
    static_assert(std::is_same_v<error_type, error<Context, error_kind>>);

    Parser parser;

    constexpr all_consuming_parser(Parser p) : parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result = parser(ctx);

        if (!result)
        {
            return result_type(rust::unexpect, std::move(ctx), result.error().code);
        }

        if (!result->first.empty())
        {
            return result_type(rust::unexpect, std::move(result->first), error_kind::eof);
        }

        return result_type(rust::in_place, std::move(result->first), std::move(result->second));
    }
};

template <typename Context, typename SepParser, typename ItemParser, bool AtLeastOne>
struct separated_list_parser
{
    using result_type1 = std::invoke_result_t<SepParser, Context>;  // iresult<Context, T1, E>
    using result_type2 = std::invoke_result_t<ItemParser, Context>;   // iresult<Context, T2, E>

    using output_type = std::vector<typename result_type2::output_type>;
    using error_type = typename result_type2::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(all_same<typename result_type1::input_type, Context, typename result_type2::input_type>);

    SepParser sep;
    ItemParser item;

    constexpr separated_list_parser(SepParser s, ItemParser i) : sep(std::move(s)), item(std::move(i)) { }

    constexpr result_type operator()(Context ctx)
    {
        output_type items;

        auto first_item = item(ctx);

        if (!first_item)
        {
            if constexpr (AtLeastOne)
            {
                return result_type(rust::unexpect, std::move(ctx), first_item.error().code);
            }
            else
            {
                return result_type(rust::in_place, std::move(ctx), std::move(items));
            }
        }

        items.emplace_back(std::move(first_item->second));
        ctx = std::move(first_item->first);

        while (true)
        {
            auto clone = ctx;
            auto sep_result = sep(clone);

            if (!sep_result)
            {
                break;
            }

            clone = std::move(sep_result->first);

            if (auto item_result = item(clone); !item_result)
            {
                break;
            }
            else
            {
                items.emplace_back(std::move(item_result->second));
                ctx = std::move(item_result->first);
            }
        }

        return result_type(rust::in_place, std::move(ctx), std::move(items));
    }
};

template <typename Context, typename Parser, bool AtLeastOne>
struct many_parser
{
    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>

    using output_type = std::vector<typename result_type1::output_type>;
    using error_type = std::conditional_t<AtLeastOne, error<Context, error_kind>, typename result_type1::error_type>;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    Parser parser;

    constexpr many_parser(Parser p) : parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        output_type items;

        auto first_item = parser(ctx);

        if (!first_item)
        {
            if constexpr (AtLeastOne)
            {
                return result_type(rust::unexpect, std::move(ctx), error_kind::many1);
            }
            else
            {
                return result_type(rust::in_place, std::move(ctx), std::move(items));
            }
        }

        items.emplace_back(std::move(first_item->second));
        ctx = std::move(first_item->first);

        while (true)
        {
            auto item_result = parser(ctx);

            if (!item_result)
            {
                break;
            }

            items.emplace_back(std::move(item_result->second));
            ctx = std::move(item_result->first);
        }

        return result_type(rust::in_place, std::move(ctx), std::move(items));
    }
};

template <typename Context, typename Parser>
struct count_parser
{
    size_t count;
    Parser parser;

    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    using output_type = std::vector<typename result_type1::output_type>;
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    constexpr count_parser(size_t c, Parser p) : count(c), parser(std::move(p)) { }

    constexpr result_type operator()(Context ctx)
    {
        output_type items;
        items.reserve(count);

        for (size_t i = 0; i < count; ++i)
        {
            auto item_result = parser(ctx);

            if (!item_result)
            {
                return result_type(rust::unexpect, std::move(ctx), item_result.error().code);
            }

            items.emplace_back(std::move(item_result->second));
            ctx = std::move(item_result->first);
        }

        return result_type(rust::in_place, std::move(ctx), std::move(items));
    }
};

// Runs the embedded parser repeatedly, filling the given slice with results.
// This parser fails if the input runs out before the given slice is full.
// Since we use an output iterator, the output container must be pre-sized.
// So we cannot check whether the given slice is full or not, we add
// another argument `sentinel`, which used for indicate whether the
// parse should be stopped.
template <typename Context, typename Parser, typename Iterator, typename Sentinel>
struct fill_parser
{
    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    using error_type = typename result_type1::error_type;
    using output_type = Iterator;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    Parser parser;
    Iterator iter;
    Sentinel sent;

    constexpr fill_parser(Parser p, Iterator i, Sentinel s) 
        : parser(std::move(p)), iter(std::move(i)), sent(std::move(s)) { }

    constexpr result_type operator()(Context ctx)
    {
        for (; iter != sent; )
        {
            auto item_result = parser(ctx);

            if (!item_result)
            {
                return result_type(rust::unexpect, std::move(ctx), item_result.error().code);
            }

            *iter++ = std::move(item_result->second);
            ctx = std::move(item_result->first);
        }

        return result_type(rust::in_place, std::move(ctx), std::move(iter));
    }
};

// Different form most design in C++, The init is a function that returns the initial value.
template <typename Context, typename Parser, typename Init, typename F, bool AtLeastOne>
struct many_fold_parser
{
    using result_type1 = std::invoke_result_t<Parser, Context>;  // iresult<Context, T1, E>
    using output_type = std::invoke_result_t<Init>;
    using error_type = typename result_type1::error_type;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    Parser parser;
    F folder;
    Init init;

    constexpr many_fold_parser(Parser p, F f, Init i) 
        : parser(std::move(p)), folder(std::move(f)), init(std::move(i)) { }

    constexpr result_type operator()(Context ctx)
    {
        using DelegateParser = many_parser<Context, std::reference_wrapper<Parser>, AtLeastOne>;
        auto result = DelegateParser(std::ref(parser))(ctx);

        if (!result)
        {
            return result_type(rust::unexpect, std::move(ctx), result.error().code);
        }

        return result_type(
            rust::in_place, 
            std::move(result->first), 
            std::ranges::fold_left(std::move(result->second), init(), std::ref(folder))
        );
    }
};

template <typename Context, typename F, typename ErrorCode>
struct replace_error_code_parser
{
    using result_type1 = std::invoke_result_t<F, Context>;  // iresult<Context, T1, E>
    using output_type = typename result_type1::output_type;
    using error_type = error<Context, ErrorCode>;
    using result_type = iresult<Context, output_type, error_type>;

    static_assert(std::is_same_v<typename result_type1::input_type, Context>);

    F parser;
    ErrorCode code;

    constexpr replace_error_code_parser(F f, ErrorCode c) : parser(std::move(f)), code(c) { }

    constexpr result_type operator()(Context ctx)
    {
        auto result = parser(ctx);

        return result ? result_type(rust::in_place, std::move(result->first), std::move(result->second))
                      : result_type(rust::unexpect, std::move(ctx), code);
    }
};


} // namespace detail

}  // namespace nom

