
#pragma once

#include <iterator>

#include "utils.hpp"
#include "error.hpp"
#include "interface.hpp"

namespace winnow::detail
{

// Returns the output of the child parser if it satisfies a verification function.
template <typename Parser, typename P>
struct verify_parser : parser_interface
{
    Parser parser;
    P predicate;

    constexpr verify_parser(Parser p, P pred) : parser(std::move(p)), predicate(std::move(pred)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using O = typename std::invoke_result_t<Parser, Stream&>::value_type;
        using R = modal_result<O, E>;

        auto result = parser(stream);

        if (!result)
        {
            return R(std::unexpect, std::move(result.error()));
        }

        if (!std::invoke(predicate, result.value()))
        {
            return make_backtrack_from_input<O>(stream);
        }

        return R(std::in_place, std::move(result.value()));
    }
};

// Maps a function over the output of a parser
template <typename Parser, typename F>
struct map_parser : parser_interface
{
    Parser parser;
    F func;

    constexpr map_parser(Parser p, F f) : parser(std::move(p)), func(std::move(f)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using R1 = std::invoke_result_t<Parser, Stream&>;
        using O1 = typename R1::value_type;
        using O2 = std::invoke_result_t<F, O1>;
        using R = modal_result<O2, E>;

        auto result = parser(stream);

        if (!result)
        {
            return R(std::unexpect, std::move(result.error()));
        }
        return R(std::in_place, std::invoke(func, std::move(result.value())));
    }
};

// Produce the provided value
template <typename Parser, typename Value>
struct value_parser : parser_interface
{
    Parser parser;
    Value value;

    constexpr value_parser(Parser p, Value v) : parser(std::move(p)), value(std::move(v)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using R = modal_result<Value, E>;

        auto result = parser(stream);

        if (!result)
        {
            return R(std::unexpect, std::move(result.error()));
        }

        return R(std::in_place, value);
    }
};


// Pick the first successful parser
template <typename... Parsers>
struct choice_parser : parser_interface
{
    static_assert(sizeof...(Parsers) > 0, "choice_parser requires at least one parser.");

    std::tuple<Parsers...> parsers;

    constexpr choice_parser(Parsers... ps) : parsers(std::move(ps)...) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using R = std::invoke_result_t<Parsers...[0], Stream&>;
        using ErrMode = err_mode<E>;

        static_assert((std::is_same_v<R, std::invoke_result_t<Parsers, Stream&>> && ...),
                      "All parsers in choice_parser must return the same result type.");

        // Just save the last error of parser.
        // For tight control over the error when no match is found, add a final case using fail.
        std::optional<ErrMode> e;  

        // C++26 support Expansion Statements P1306R5.
        template for (const auto& parser : parsers)
        {
            auto clone = stream;
            auto result = parser(clone);

            if (result)
            {
                stream = std::move(clone);
                return result;
            }
            else if (result.error().is_cut())
            {
                // Stop parsing further as a cut has been encountered.
                return result;
            }
            
            e.emplace(std::move(result.error()));
        }

        // The e must have value since we require at least one parser in choice_parser.
        return R(std::unexpect, std::move(e.value()));
    }

};

// Recognizes a literal
template <typename CharT>
struct literal_parser : parser_interface
{
    using literal_type = std::basic_string_view<CharT>;

    literal_type constant;

    constexpr literal_parser(literal_type t) : constant(t) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        // Rust winnow return a part of input/stream. 
        // We just return slices of the input stream.
        using E = typename Stream::error_type;
        using O = literal_type;
        using R = modal_result<literal_type, E>;

        if (stream.match(constant, false))
        {
            auto [left, right] = stream.split_at(constant.size());
            stream = std::move(right);
            return R(std::in_place, std::move(left));
        }
        else
        {
            return make_backtrack_from_input<O>(stream);
        }
    }
};

// Recognize the longest input slice 
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
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = modal_result<O, E>;
        
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
            // It will return an ErrMode::Backtrack(_) if the 
            // set of tokens wasn’t met or is out of occurrences range.
            return make_backtrack_from_input<O>(stream);
        }
        else
        {
            auto [left, right] = stream.split_at(count);
            stream = std::move(right);
            return R(std::in_place, std::move(left));
        }
    }
        
};

// Recognize an input slice containing the first N input elements (I[..N]).
struct take_parser : parser_interface
{
    size_t count;

    constexpr take_parser(size_t n) : count(n) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = modal_result<O, E>;

        if (stream.size() < count)
        {
            // It will return Err(ErrMode::Backtrack(_)) if the input is shorter than the argument
            return make_backtrack_from_input<O>(stream);
        }
        else
        {
            auto [left, right] = stream.split_at(count);
            stream = std::move(right);
            return R(std::in_place, std::move(left));
        }
    }
};

// Recognize the longest input slice (bound by occurrences) till a member of a set of tokens is found.
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
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = modal_result<O, E>;

        const auto idx = stream.to_string_view().find(value);

        // It will return an ErrMode::Backtrack(_) if the set of 
        // tokens wasn’t met or is out of occurrences range.
        if (idx == literal_type::npos || !range.contains(idx))
        {
            return make_backtrack_from_input<O>(stream);
        }

        auto [left, right] = stream.split_at(idx);
        stream = std::move(right);
        return R(std::in_place, std::move(left));
    }
};

// Sequence two parsers, only returning the output from the second.
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
        using E = typename Stream::error_type;
        using R1 = std::invoke_result_t<IgnoredParser, Stream&>;
        using R2 = std::invoke_result_t<Parser, Stream&>;
        using R = R2;
        static_assert(std::is_same_v<R1, R2>, "The result types of the main parser and the ignored parser must be the same.");

        auto ignored_result = ignored_parser(stream);

        if (!ignored_result)
        {
            // If the ignored parser fails, we return an unexpected result for the main parser.
            return R(std::unexpect, std::move(ignored_result.error()));
        }
        return R(parser(stream));
    }

};

// Sequence two parsers, only returning the output of the first.
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
        using E = typename Stream::error_type;
        using R1 = std::invoke_result_t<Parser, Stream&>;
        using R2 = std::invoke_result_t<IgnoredParser, Stream&>;
        using R = R1;
        static_assert(std::is_same_v<R1, R2>, "The result types of the main parser and the ignored parser must be the same.");

        auto result = parser(stream);

        if (!result)
        {
            // If the main parser fails, we return an unexpected result.
            return R(std::unexpect, std::move(result.error()));
        }
        auto ignored_result = ignored_parser(stream);

        if (!ignored_result)
        {
            // If the ignored parser fails, we return an unexpected result for the main parser.
            return R(std::unexpect, std::move(ignored_result.error()));
        }

        return R(std::in_place, std::move(result.value()));
    }
};

// Sequence three parsers, only returning the values of the first and third.
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
        using R1 = std::invoke_result_t<Parser1, Stream&>;
        using R2 = std::invoke_result_t<SepParser, Stream&>;
        using R3 = std::invoke_result_t<Parser2, Stream&>;
        using E = typename Stream::error_type;

        using O1 = typename R1::value_type;
        using O2 = typename R2::value_type;
        using O = std::pair<O1, O2>;
        using R = modal_result<O, E>;

        auto result1 = parser1(stream);

        if (!result1)
        {
            return R(std::unexpect, std::move(result1.error()));
        }
        auto sep_result = sep_parser(stream);

        if (!sep_result)
        {
            return R(std::unexpect, std::move(sep_result.error()));
        }
        auto result2 = parser2(stream);

        if (!result2)
        {
            return R(std::unexpect, std::move(result2.error()));
        }

        // Only all parsers succeed do we return a successful result.
        return R(std::in_place, std::make_pair(std::move(result1.value()), std::move(result2.value())));
    }
};

struct rest_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = modal_result<O, E>;

        auto left = stream.to_string_view();
        stream.advance(stream.size());
        return R(std::in_place, std::move(left));
    }
};

struct rest_len_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using O = size_t;
        using R = modal_result<O, E>;
        return R(std::in_place, stream.size());
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
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = modal_result<O, E>;

        if (stream.size() == 0 || std::invoke(pred, stream[0]))
        {
            return R(std::unexpect,
                err_mode<E>::make_backtrack(
                    error_traits<E>::from_input(stream)
                )
            );
        }
        auto [left, right] = stream.split_at(1);
        stream = std::move(right);
        return R(std::in_place, std::move(left));
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
            result.error().switch_to_backtrack();
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
            result.error().switch_to_cut();
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

        using R1 = std::invoke_result_t<Parser, Stream&>;
        using E = typename Stream::error_type;
        using O = std::optional<typename R1::value_type>;
        using R = modal_result<O, E>;

        if (result)
        {
            return R(std::in_place, std::make_optional(std::move(result.value())));
        }
        else if (result.error().is_backtrack())
        {
            // Only backtrack errors should reset the stream to the clone.
            stream = std::move(clone);
            return R(std::in_place, std::nullopt);
        }
        else
        {
            // For cut, just stop and propagate the error.
            return R(std::unexpect, result.error());
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

        using O = unit;
        using E = typename Stream::error_type;
        using R = modal_result<O, E>;

        if (result)
        {
            return make_backtrack_from_input<O>(stream);
        }

        if (result.error().is_backtrack())
        {
            return R(std::in_place, unit{});
        }
        else
        {
            // For cut errors, just propagate the error without backtracking.
            return R(std::unexpect, result.error());
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
        auto collector = accumulator.initial();
        size_t count = 0;
        size_t size = stream.size();
        
        using E = typename Stream::error_type;
        using O = decltype(collector);
        using R = modal_result<O, E>;

        while (stream.size())
        {
            auto result = parser(stream);

            if (!result) 
            {
                if (result.error().is_cut())
                {
                    return R(std::unexpect, result.error());
                }
                break;
            }

            accumulator.accumulate(collector, std::move(result.value()));
            ++count;

            if (size == stream.size())
            {
                // Current loop will not consume any input, avoid infinite loop
                return make_backtrack_from_input<O>(stream);
            }

            if (range.is_over(count))
            {
                break;
            }
        }

        if (range.is_under(count))
        {
            return make_backtrack_from_input<O>(stream);
        }

        return R(std::in_place, std::move(collector));
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
        auto results = accumulator.initial();

        using E = typename Stream::error_type;
        using O = decltype(results);
        using R = modal_result<decltype(results), E>;

        // For empty stream, return empty list
        if (stream.size())
        {   
            auto clone = stream;
            auto first = parser(clone);

            // If the first item cannot be parsed, handle the error or 
            // return an empty result if allowed by the range
            if (!first)
            {
                if (first.error().is_cut())
                {
                    return R(std::unexpect, first.error());
                }

                if (range.contains(results.size()))
                {
                    return R(std::in_place, std::move(results));
                }
                else
                {
                    return make_backtrack_from_input<O>(stream);
                }
            }

            accumulator.accumulate(results, std::move(first.value()));
            stream = std::move(clone);

            if (range.is_upper_bound(results.size()))
            {
                return R(std::in_place, std::move(results));
            }

            while (stream.size())
            {
                auto clone = stream;
                auto sep = separator(clone);

                if (!sep)
                {
                    if (sep.error().is_cut())
                    {
                        return R(std::unexpect, sep.error());
                    }
                    // Stop parsing if the separator is not found
                    stream = std::move(clone);
                    break;
                }

                auto item = parser(clone);

                if (!item)
                {
                    if (item.error().is_cut())
                    {
                        return R(std::unexpect, item.error());
                    }
                    // Stop parsing if the item is not found
                    // stream = std::move(clone);
                    break;
                }

                accumulator.accumulate(results, std::move(item.value()));
                stream = std::move(clone);

                if (range.is_upper_bound(results.size()))
                {
                    return R(std::in_place, std::move(results));
                }

            }
        }

        if (!range.contains(results.size()))
        // if (!range.is_under(results.size()))
        {
            return make_backtrack_from_input<O>(stream);
        }

        return R(std::in_place, std::move(results));
    }
};

template <typename Parser>
struct cond_parser : parser_interface
{
    bool condition;
    Parser parser;

    constexpr cond_parser(bool condition, Parser parser)
        : condition(condition), parser(std::move(parser)) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using O = std::optional<std::invoke_result_t<Parser, Stream&>>;
        using R = modal_result<O, E>;

        if (condition)
        {
            return parser(stream).transform([](auto x) -> O { return std::make_optional(std::move(x)); });
        }

        return R(std::in_place, std::nullopt);
    }
};

struct empty_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using R = modal_result<unit, E>;
        return R(std::in_place, unit{});
    }
};

template <typename O>
struct fail_parser : parser_interface
{
    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using R = modal_result<O, E>;
        return make_backtrack_from_input<O>(stream);
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
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = modal_result<O, E>;

        if (stream.empty())
        {
            return R(std::in_place, "");
        }
        else
        {
            return make_backtrack_from_input<O>(stream);
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
            return !cached_value.has_value(); 
        }

        constexpr reference operator*() const 
        { 
            return cached_value.value();
        }
        
        constexpr iterator& operator++() 
        { 
            if (cached_value.has_value())
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

// Accumulate the output of parser f into a container until the 
// parser g produces a result (bound by occurrences).
template <typename Parser, typename TerminatorParser, typename Accumulator>
struct repeat_till_parser
{
    Parser parser;
    TerminatorParser terminator_parser;
    [[no_unique_address]] Accumulator accumulator;
    occurrences<size_t> range;

    constexpr repeat_till_parser(Parser p, TerminatorParser tp, Accumulator acc, occurrences<size_t> r)
        : parser(std::move(p)), terminator_parser(std::move(tp)), accumulator(std::move(acc)), range(r) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        auto results = accumulator.initial();

        using E = typename Stream::error_type;
        using O = decltype(results);
        using R = modal_result<decltype(results), E>;

        size_t count = 0;

        throw std::logic_error("Not implemented error.");
    }

};

}  // namespace winnow::detail


