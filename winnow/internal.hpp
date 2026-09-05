
#pragma once

#include <iterator>

#include "utils.hpp"
#include "error.hpp"

namespace winnow::detail
{

template <typename Parser, typename F> struct map_parser;

template <typename Parser, typename P> struct verify_parser;

template <typename Parser, typename Value> struct value_parser;

struct parser_interface
{
    template <typename Self, typename F>
    constexpr auto map(this Self&& self, F&& f)
    {
        return map_parser<std::decay_t<Self>, F>{std::forward<Self>(self), std::forward<F>(f)};
    }

    template <typename Self, typename P>
    constexpr auto verify(this Self&& self, P&& p)
    {
        return verify_parser<std::decay_t<Self>, P>{std::forward<Self>(self), std::forward<P>(p)};
    }

    template <typename Self, typename Value>
    constexpr auto value(this Self&& self, Value&& value)
    {
        return value_parser<std::decay_t<Self>, Value>{std::forward<Self>(self), std::forward<Value>(value)};
    }
};

template <typename Parser, typename P>
struct verify_parser : parser_interface
{
    Parser parser;
    P predicate;

    constexpr verify_parser(Parser p, P pred) : parser(std::move(p)), predicate(std::move(pred)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;
        using result_type = std::invoke_result_t<Parser, Stream&>;

        auto result = parser(stream);

        if (!result)
        {
            return result_type::make_err(std::move(result.unwrap_err()));
        }

        if (!std::invoke(predicate, result.unwrap_ok()))
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }

        return result_type::make_ok(std::move(result.unwrap_ok()));
    }
};

template <typename Parser, typename F>
struct map_parser : parser_interface
{
    Parser parser;
    F func;

    constexpr map_parser(Parser p, F f) : parser(std::move(p)), func(std::move(f)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using result_type1 = std::invoke_result_t<Parser, Stream&>;
        using output_type1 = typename result_type1::value_type;
        using error_type = typename Stream::error_type;
        using output_type = std::invoke_result_t<F, output_type1>;
        using result_type = modal_result<output_type, error_type>;

        auto result = parser(stream);

        if (!result)
        {
            return result_type::make_err(std::move(result.unwrap_err()));
        }
        return result_type::make_ok(std::invoke(func, std::move(result.unwrap_ok())));
    }
};

template <typename Parser, typename Value>
struct value_parser : parser_interface
{
    Parser parser;
    Value value;

    constexpr value_parser(Parser p, Value v) : parser(std::move(p)), value(std::move(v)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using result_type1 = std::invoke_result_t<Parser, Stream&>;
        using output_type1 = typename result_type1::value_type;
        using error_type = typename Stream::error_type;
        using result_type = modal_result<Value, error_type>;

        auto result = parser(stream);

        if (!result)
        {
            return result_type::make_err(std::move(result.unwrap_err()));
        }

        return result_type::make_ok(value);
    }
};

template <typename... Parsers>
struct choice_parser : parser_interface
{
    static_assert(sizeof...(Parsers) > 0, "choice_parser requires at least one parser.");

    std::tuple<Parsers...> parsers;

    constexpr choice_parser(Parsers... ps) : parsers(std::move(ps)...) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using result_type = std::invoke_result_t<Parsers...[0], Stream&>;

        static_assert((std::is_same_v<result_type, std::invoke_result_t<Parsers, Stream&>> && ...),
                      "All parsers in choice_parser must return the same result type.");

        using error_type = typename Stream::error_type;

        std::optional<err_mode<error_type>> e;

        template for (const auto& parser : parsers)
        {
            auto clone = stream;
            auto result = parser(clone);

            if (result)
            {
                stream = std::move(clone);
                return result;
            }
            else if (result.unwrap_err().is_cut())
            {
                return result;
            }
            
            e.emplace(std::move(result.unwrap_err()));
        }

        // The e must have value since we require at least one parser in choice_parser.
        return result_type::make_err(std::move(e.value()));
    }

};

template <typename CharT>
struct literal_parser : parser_interface
{

    using literal_type = std::basic_string_view<CharT>;

    literal_type constant;

    constexpr literal_parser(literal_type t) : constant(t) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<literal_type, error_type>;

        if (stream.match(constant, false))
        {
            auto [left, right] = stream.split_at(constant.size());
            stream = std::move(right);
            return result_type::make_ok(std::move(left));
        }
        else
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }
    }
};

template <typename Pred>
struct take_while_parser : parser_interface
{
    Pred pred;
    occurrences<size_t> range;

    constexpr take_while_parser(Pred p, occurrences<size_t> r)
        : pred(p), range(r) { }
    
    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<std::basic_string_view<typename Stream::value_type>, error_type>;
        
        // User should ensure that the upper is not less than lower.
        size_t count = 0;

        while (count < stream.size() && std::invoke(pred, stream[count]))
        {
            if (range.is_upper_bound(count))
            {
                break;
            }
            ++count;
        }

        if (range.is_under(count))
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }
        else
        {
            auto [left, right] = stream.split_at(count);
            stream = std::move(right);
            return result_type::make_ok(std::move(left));
        }
    }
        
};

struct take_parser : parser_interface
{
    size_t count;

    constexpr take_parser(size_t n) : count(n) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<std::basic_string_view<typename Stream::value_type>, error_type>;

        if (stream.size() < count)
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }
        else
        {
            auto [left, right] = stream.split_at(count);
            stream = std::move(right);
            return result_type::make_ok(std::move(left));
        }
    }
};

template <typename CharT>
struct take_until_parser : parser_interface
{
    using literal_type = std::basic_string_view<CharT>;
    
    literal_type value;
    occurrences<size_t> range;

    constexpr take_until_parser(literal_type v, occurrences<size_t> r)
        : value(v), range(r) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<literal_type, error_type>;

        const auto idx = stream.to_string_view().find(value);

        if (idx == literal_type::npos || !range.contains(idx))
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }

        auto [left, right] = stream.split_at(idx);
        stream = std::move(right);
        return result_type::make_ok(std::move(left));
    }
};

template <typename IgnoredParser, typename Parser>
struct preceded_parser : parser_interface
{
    IgnoredParser ignored_parser;
    Parser parser;

    constexpr preceded_parser(IgnoredParser ip, Parser p) 
        : ignored_parser(std::move(ip)), parser(std::move(p)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using result_type1 = std::invoke_result_t<IgnoredParser, Stream&>;
        using result_type2 = std::invoke_result_t<Parser, Stream&>;    
        
        using error_type1 = typename result_type1::error_type;
        using error_type2 = typename result_type2::error_type;

        using error_type = typename Stream::error_type;
        using result_type = result_type2;

        auto ignored_result = ignored_parser(stream);

        if (!ignored_result)
        {
            return result_type::make_err(std::move(ignored_result.unwrap_err()));
        }
        return parser(stream);
    }

};

template <typename Parser, typename IgnoredParser>
struct terminated_parser : parser_interface
{
    Parser parser;
    IgnoredParser ignored_parser;

    constexpr terminated_parser(Parser p, IgnoredParser ip) 
        : parser(std::move(p)), ignored_parser(std::move(ip)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using result_type1 = std::invoke_result_t<Parser, Stream&>;
        using result_type2 = std::invoke_result_t<IgnoredParser, Stream&>;    
        
        using error_type1 = typename result_type1::error_type;
        using error_type2 = typename result_type2::error_type;

        using error_type = typename Stream::error_type;
        using result_type = result_type1;

        auto result = parser(stream);

        if (!result)
        {
            return result_type::make_err(std::move(result.unwrap_err()));
        }
        auto ignored_result = ignored_parser(stream);

        if (!ignored_result)
        {
            return result_type::make_err(std::move(ignored_result.unwrap_err()));
        }
        return result;
    }
};

template <typename Parser1, typename SepParser, typename Parser2>
struct separated_pair_parser : parser_interface
{
    Parser1 parser1;
    SepParser sep_parser;
    Parser2 parser2;

    constexpr separated_pair_parser(Parser1 p1, SepParser sp, Parser2 p2) 
        : parser1(std::move(p1)), sep_parser(std::move(sp)), parser2(std::move(p2)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using result_type1 = std::invoke_result_t<Parser1, Stream&>;
        using result_type2 = std::invoke_result_t<Parser2, Stream&>;

        using error_type = typename Stream::error_type;
        using output_type = std::pair<typename result_type1::value_type, typename result_type2::value_type>;
        using result_type = modal_result<output_type, error_type>;

        auto result1 = parser1(stream);

        if (!result1)
        {
            return result_type::make_err(std::move(result1.unwrap_err()));
        }
        auto sep_result = sep_parser(stream);

        if (!sep_result)
        {
            return result_type::make_err(std::move(sep_result.unwrap_err()));
        }
        auto result2 = parser2(stream);

        if (!result2)
        {
            return result_type::make_err(std::move(result2.unwrap_err()));
        }
        return result_type::make_ok(std::make_pair(std::move(result1.unwrap_ok()), std::move(result2.unwrap_ok())));
    }
};

struct rest_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using literal_type = std::basic_string_view<typename Stream::value_type>;
        using error_type = typename Stream::error_type;
        using result_type = modal_result<literal_type, error_type>;

        auto left = stream.to_string_view();
        stream.advance(stream.size());
        return result_type::make_ok(left);
    }
};

struct rest_len_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<size_t, error_type>;
        return result_type::make_ok(stream.size());
    }
};

struct always_false
{
    template <typename... Ts>
    static constexpr bool operator()(Ts&&...) { return false; }
};

template <typename Pred>
struct check_next_character_parser : parser_interface
{
    Pred pred;

    constexpr check_next_character_parser(Pred pred) : pred(std::move(pred)) {}

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using literal_type = std::basic_string_view<typename Stream::value_type>;
        using error_type = typename Stream::error_type;
        using result_type = modal_result<literal_type, error_type>;

        // if (stream.size() == 0 || !std::ranges::contains(tokens, stream[0]))
        if (stream.size() == 0 || std::invoke(pred, stream[0]))
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }
        auto [left, right] = stream.split_at(1);
        stream = std::move(right);
        return result_type::make_ok(std::move(left));
    }
};

template <typename Parser>
struct backtrack_err_parser : parser_interface
{
    Parser parser;

    constexpr backtrack_err_parser(Parser p) : parser(std::move(p)) {}

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        auto result = parser(stream);

        if (!result) 
        {
            result.unwrap_err().switch_to_backtrack();
        }

        return result;
    }
};

template <typename Parser>
struct cut_err_parser : parser_interface
{
    Parser parser;

    constexpr cut_err_parser(Parser p) : parser(std::move(p)) {}

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        auto result = parser(stream);

        if (!result) 
        {
            result.unwrap_err().switch_to_cut();
        }

        return result;
    }
};

template <typename Parser>
struct peek_parser : parser_interface
{
    Parser parser;

    constexpr peek_parser(Parser p) : parser(std::move(p)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        auto clone = stream;
        return parser(clone);
    }
};

template <typename Parser>
struct opt_parser : parser_interface
{
    Parser parser;

    constexpr opt_parser(Parser p) : parser(std::move(p)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        auto clone = stream;
        auto result = parser(stream);

        using inner_type = typename decltype(result)::value_type;
        using error_type = typename Stream::error_type;
        using result_type = modal_result<std::optional<inner_type>, error_type>;

        if (result)
        {
            return result_type::make_ok(std::make_optional(std::move(result.unwrap_ok())));
        }
        else if (result.unwrap_err().is_backtrack())
        {
            // Only backtrack errors should reset the stream to the clone.
            stream = std::move(clone);
            return result_type::make_ok(std::nullopt);
        }
        else
        {
            return result_type::make_err(
                err_mode<error_type>::make_cut(result.unwrap_err().as_cut())
            );
        }
    }
};

template <typename Parser>
struct not_parser : parser_interface
{
    Parser parser;

    constexpr not_parser(Parser p) : parser(std::move(p)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        auto clone = stream;
        auto result = parser(clone);

        using value_type = std::tuple<>;
        using error_type = typename Stream::error_type;
        using result_type = modal_result<value_type, error_type>;

        if (result)
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }

        if (result.unwrap_err().is_backtrack())
        {
            return result_type::make_ok();
        }
        else
        {
            return result_type::make_err(
                err_mode<error_type>::make_cut(result.unwrap_err().as_cut())
            );
        }

    }
};

template <typename Accumulator, typename Parser>
struct repeat_parser : parser_interface
{
    Parser parser;
    [[no_unique_address]] Accumulator accumulator;
    occurrences<size_t> range;

    constexpr repeat_parser(Parser p, Accumulator o, occurrences<size_t> r)
        : parser(std::move(p)), accumulator(std::move(o)), range(std::move(r)) {}

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using inner = std::invoke_result_t<Parser, Stream&>;
        using value_type = typename inner::value_type;
        using error_type = typename Stream::error_type;

        auto collector = accumulator.initial();
        size_t count = 0;
        size_t size = stream.size();
        
        using result_type = modal_result<decltype(collector), error_type>;

        while (stream.size())
        {
            auto result = parser(stream);

            if (!result) 
            {
                if (result.unwrap_err().is_cut())
                {
                    return result_type::make_err(
                        err_mode<error_type>::make_cut(result.unwrap_err().as_cut())
                    );
                }
                break;
            }

            accumulator.accumulate(collector, std::move(result.unwrap_ok()));
            ++count;

            if (size == stream.size())
            {
                // Current loop will not consume any input, avoid infinite loop
                return result_type::make_err(
                    err_mode<error_type>::make_backtrack(
                        error_traits<error_type>::from_input(stream)
                    )
                );
            }

            if (range.is_over(count))
            {
                break;
            }
        }

        if (range.is_under(count))
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }

        return result_type::make_ok(std::move(collector));
    }
};

template <typename Parser, typename Sep, typename Accumulator>
struct separated_parser : parser_interface
{
    Parser parser;
    [[no_unique_address]] Sep separator;
    [[no_unique_address]] Accumulator accumulator;
    occurrences<size_t> range; 

    constexpr separated_parser(Parser p, Sep s, Accumulator acc, occurrences<size_t> r)
        : parser(std::move(p)), separator(std::move(s)), accumulator(std::move(acc)), range(r) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;

        auto results = accumulator.initial();

        using result_type = modal_result<decltype(results), error_type>;

        // For empty stream, return empty list
        if (stream.size())
        {   
            auto clone = stream;
            auto first = parser(clone);

            // If the first item cannot be parsed, handle the error or 
            // return an empty result if allowed by the range
            if (!first)
            {
                if (first.unwrap_err().is_cut())
                {
                    return result_type::make_err(
                        err_mode<error_type>::make_cut(first.unwrap_err().as_cut())
                    );
                }

                if (range.contains(results.size()))
                {
                    return result_type::make_ok(std::move(results));
                }
                else
                {
                    return result_type::make_err(
                        err_mode<error_type>::make_backtrack(
                            error_traits<error_type>::from_input(stream)
                        )
                    );
                }
            }

            accumulator.accumulate(results, std::move(first.unwrap_ok()));
            stream = std::move(clone);

            if (range.is_upper_bound(results.size()))
            {
                return result_type::make_ok(std::move(results));
            }

            while (stream.size())
            {
                auto clone = stream;
                auto sep = separator(clone);

                if (!sep)
                {
                    if (sep.unwrap_err().is_cut())
                    {
                        return result_type::make_err(
                            err_mode<error_type>::make_cut(sep.unwrap_err().as_cut())
                        );
                    }
                    // Stop parsing if the separator is not found
                    stream = std::move(clone);
                    break;
                }

                auto item = parser(clone);

                if (!item)
                {
                    if (item.unwrap_err().is_cut())
                    {
                        return result_type::make_err(
                            err_mode<error_type>::make_cut(item.unwrap_err().as_cut())
                        );
                    }
                    // Stop parsing if the item is not found
                    // stream = std::move(clone);
                    break;
                }

                accumulator.accumulate(results, std::move(item.unwrap_ok()));
                stream = std::move(clone);

                if (range.is_upper_bound(results.size()))
                {
                    return result_type::make_ok(std::move(results));
                }

            }
        }

        if (!range.contains(results.size()))
        // if (!range.is_under(results.size()))
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }

        return result_type::make_ok(std::move(results));
    }
};

// template <typename Parser>
// struct cond_parser : parser_interface
// {
//     bool condition;
//     Parser parser;

//     constexpr cond_parser(bool condition, Parser parser)
//         : condition(condition), parser(std::move(parser)) { }

//     template <typename Stream>
//     constexpr auto operator()(Stream& stream) -> modal_result<std::optional<std::invoke_result_t<Parser, Stream&>>, typename Stream::error_type> 
//     {
//         using invoke_type = std::invoke_result_t<Parser, Stream&>;
//         using error_type = typename Stream::error_type;
//         using result_type = modal_result<std::optional<invoke_type>, error_type>;

//         if (condition)
//         {
//             auto result = parser(stream).map([](auto x) -> std::optional<invoke_type> { return std::make_optional(std::move(x)); });
//             return result;
//             // return result 
//                 //  ? result_type::make_ok(std::move(result.unwrap_ok()))
//                 //  : result_type::make_err(std::move(result.unwrap_err()));
//         }

//         return result_type::make_ok(std::nullopt); 
//     }
// };

struct empty_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<std::tuple<>, error_type>;
        return result_type::make_ok(std::tuple<>());
    }
};

template <typename O>
struct fail_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<O, error_type>;

        return result_type::make_err(
            err_mode<error_type>::make_backtrack(
                error_traits<error_type>::from_input(stream)
            )
        );
    }
};

template <typename Output>
struct fail_fn
{
    template <typename Message>
    static constexpr auto operator()(const Message&)
    {
        return fail_parser<Output>();
    }
};

struct eof_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using error_type = typename Stream::error_type;
        using value_type = typename Stream::value_type;
        using result_type = modal_result<std::basic_string_view<value_type>, error_type>;

        if (stream.empty())
        {
            return result_type::make_ok("");
        }
        else
        {
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }
    }
};

// How to raise an error when the result is Cut<E>?
// This iterator contains a field `cached_value` which 
// stores the most recent result of the parser.
// If the parser meets a Cut<E> result, user can access
// it through the `cached_value` field.
template <typename Stream, typename Parser>
struct iterator_parser
{
    using input_type = Stream;
    using result_type = std::invoke_result_t<Parser, Stream&>;  // modal_result<O, E>

    struct iterator
    {
        using value_type = typename result_type::value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using reference = const value_type&;

        iterator_parser* base;
        result_type cached_value;

        constexpr iterator(iterator_parser* b, result_type result) : base(b), cached_value(std::move(result)) {  }

        constexpr iterator(const iterator& other) = default;

        constexpr iterator& operator=(const iterator& other) = default;

        constexpr bool operator==(std::default_sentinel_t) const 
        { 
            return !cached_value.is_ok(); 
        }

        constexpr reference operator*() const 
        { 
            return cached_value.unwrap_ok();
        }
        
        constexpr iterator& operator++() 
        { 
            if (cached_value.is_ok())
            {
                cached_value = std::invoke(base->parser, base->stream);
            }
            return *this; 
        }

        constexpr iterator operator++(int) 
        { 
            auto temp = *this; 
            ++*this; 
            return temp; 
        }
    };

    using sentinel = std::default_sentinel_t;

    constexpr iterator begin() 
    {
        auto result = parser(stream);
        return iterator(this, std::move(result));        
    }

    constexpr sentinel end() { return sentinel(); }

    constexpr iterator_parser(Stream& c, Parser p) : stream(c), parser(std::move(p)) { }

    Stream& stream;
    Parser parser;
};



}  // namespace winnow::detail


