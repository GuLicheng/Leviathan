#pragma once

#include <leviathan/config_parser/core/parser_interface.hpp>
#include <leviathan/config_parser/core/utils.hpp>
#include <leviathan/config_parser/core/error.hpp>
#include <utility>
#include <optional>
#include <functional>

namespace cpp::config::parser::detail
{

template <typename Parser1, typename Parser2> 
class and_then_parser : public parser_interface
{
    [[no_unique_address]] Parser1 m_parser1;
    [[no_unique_address]] Parser2 m_parser2;

public:

    static constexpr bool is_always_succeed = false;

    template <typename P1, typename P2>
    constexpr and_then_parser(P1&& p1, P2&& p2)
        : m_parser1((P1&&) p1), m_parser2((P2&&) p2)
    { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using R = std::invoke_result_t<Parser2, Stream&>;

        auto result1 = m_parser1(stream);
        
        if (!result1)
        {
            return make_recoverable_from_input<R>(stream);
        }

        // The output produced by a parser must be constructible into a Stream.
        Stream stream2(result1.value());
        auto result2 = m_parser2(stream2);

        if (!result2)
        {
            return make_recoverable_from_input<R>(stream);
        }

        return R(std::in_place, std::move(result2.value()));
    }
};

template <typename Parser1, typename Parser2>
and_then_parser(Parser1&&, Parser2&&) -> and_then_parser<std::decay_t<Parser1>, std::decay_t<Parser2>>;

template <typename Parser, typename F>
class verify_parser : public parser_interface
{
    [[no_unique_address]] Parser m_parser;
    [[no_unique_address]] F m_func;

public:

    static constexpr bool is_always_succeed = false;

    template <typename Parser2, typename F2>
    constexpr verify_parser(Parser2&& p, F2&& f)
        : m_parser((Parser2&&) p), m_func((F2&&) f)
    { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        auto clone = stream;
        auto result = m_parser(clone);

        using R = decltype(result);

        if (!result)
        {
            return R(std::unexpect, std::move(result.error()));
        }

        if (!std::invoke(m_func, result.value()))
        {   
            return make_recoverable_from_input<R>(stream);
        }

        // Only update the original stream if the verification succeeds.
        stream = std::move(clone);
        return R(std::in_place, std::move(result.value()));
    }
};

template <typename Parser, typename F>
verify_parser(Parser&&, F&&) -> verify_parser<std::decay_t<Parser>, std::decay_t<F>>;

template <typename Parser, typename F> 
class map_result_parser : public parser_interface
{
    [[no_unique_address]] Parser m_parser;
    [[no_unique_address]] F m_func;

public:

    static constexpr bool is_always_succeed = false;

    template <typename Parser2, typename F2>
    constexpr map_result_parser(Parser2&& p, F2&& f)
        : m_parser((Parser2&&) p), m_func((F2&&) f)
    { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        return std::invoke(m_func, std::invoke(m_parser, stream));
    }
    template <typename Stream>
    constexpr auto operator()(Stream& stream) 
    {
        return std::invoke(m_func, std::invoke(m_parser, stream));
    }
};

template <typename Parser, typename F>
map_result_parser(Parser&&, F&&) -> map_result_parser<std::decay_t<Parser>, std::decay_t<F>>;

template <typename Parser>
class warpper_parser : public parser_interface
{
    [[no_unique_address]] Parser m_parser;

public:

    static constexpr bool is_always_succeed = false;

    template <typename Parser2>
    constexpr warpper_parser(Parser2&& p)
        : m_parser((Parser2&&) p)
    { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        return m_parser(stream);
    }
};

template <typename Parser>
warpper_parser(Parser&&) -> warpper_parser<std::decay_t<Parser>>;

template <typename CharT>
class take_until_parser : public parser_interface
{
    using literal_type = std::basic_string_view<CharT>;
    
    literal_type m_value;
    occurrences<size_t> m_range;

public:

    static constexpr bool is_always_succeed = false;

    constexpr take_until_parser(literal_type v, occurrences<size_t> r)
        : m_value(v), m_range(r) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = parse_result<O, E>;

        const auto idx = stream.to_string_view().find(m_value);

        // It will return an ErrMode::Backtrack(_) if the set of 
        // tokens wasn’t met or is out of occurrences range.
        if (idx == literal_type::npos || !m_range.is_within(idx))
        {
            return make_recoverable_from_input<R>(stream);
        }

        return R(std::in_place, stream.advance_and_discard(idx));
    }
};

template <typename CharT>
class literal_parser : public parser_interface
{
    using literal_type = std::basic_string_view<CharT>;

    literal_type m_constant;

public:

    static constexpr bool is_always_succeed = false;

    constexpr literal_parser(literal_type t) : m_constant(t) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        // Rust winnow return a part of input/stream. 
        // We just return slices of the input stream.
        using E = typename Stream::error_type;
        using O = literal_type;
        using R = parse_result<literal_type, E>;

        return stream.match(m_constant, false)
             ? R(std::in_place, stream.advance_and_discard(m_constant.size()))
             : make_recoverable_from_input<R>(stream);
    }
};

template <typename Pred>
class take_while_parser : public parser_interface
{
    [[no_unique_address]] Pred m_pred;
    occurrences<size_t> m_range;

public:

    static constexpr bool is_always_succeed = false;

    template <typename Pred2>
    constexpr take_while_parser(Pred2&& p, occurrences<size_t> r)
        : m_pred((Pred2&&) p), m_range(r) { }
    
    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = parse_result<O, E>;
        
        // User should ensure that the upper is not less than lower.
        size_t count = 0;

        while (count < stream.size() && std::invoke(m_pred, stream[count]))
        {
            if (m_range.is_upper_bound(count))
            {
                break;
            }
            ++count;
        }

        if (m_range.is_less_than_lower(count))
        {
            return make_recoverable_from_input<R>(stream);
        }
        else
        {
            return R(std::in_place, stream.advance_and_discard(count));
        }
    }
};

template <typename Pred>
take_while_parser(Pred&&, occurrences<size_t>) -> take_while_parser<std::decay_t<Pred>>;

template <typename... Parsers>
class alternative_parser : public parser_interface
{
    static_assert(sizeof...(Parsers) > 0, "alternative_parser requires at least one parser.");

    [[no_unique_address]] std::tuple<Parsers...> m_parsers;

public:

    static constexpr bool is_always_succeed = false;

    template <typename... Ps>
    constexpr alternative_parser(Ps&&... ps) : m_parsers((Ps&&)ps...) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using R = std::invoke_result_t<Parsers...[0], Stream&>;
        using ErrMode = err_mode<E>;

        static_assert((std::is_same_v<R, std::invoke_result_t<Parsers, Stream&>> && ...),
                      "All parsers in alternative_parser must return the same result type.");

        // Just save the last error of parser.
        // For tight control over the error when no match is found, add a final case using fail.
        std::optional<ErrMode> e;  

        // C++26 support Expansion Statements P1306R5.
        template for (const auto& parser : m_parsers)
        {
            auto clone = stream;
            auto result = parser(clone);

            if (result)
            {
                stream = std::move(clone);
                return result;
            }
            else if (result.error().is_fatal())
            {
                // Stop parsing further as a cut has been encountered.
                return result;
            }
            
            e.emplace(std::move(result.error()));
        }

        // The e must have value since we require at least one parser in alternative_parser.
        return R(std::unexpect, std::move(e.value()));
    }

};

template <typename... Parsers>
alternative_parser(Parsers&&... ps) -> alternative_parser<std::decay_t<Parsers>...>;

template <typename... Parsers>
class sequence_parser : public parser_interface
{
    std::tuple<Parsers...> m_parsers;

    static constexpr auto indices = std::make_index_sequence<sizeof...(Parsers)>{};

public:

    static constexpr bool is_always_succeed = false;


    // auto p1 = sequence(...)
    // auto p2 = p1; // error why?
    template <typename... Parser2>
    constexpr sequence_parser(Parser2&&... ps) : m_parsers((Parser2&&) ps...) { } 

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using OptOs = std::tuple<std::optional<typename std::invoke_result_t<Parsers, Stream&>::value_type>...>;
        using Os = std::tuple<typename std::invoke_result_t<Parsers, Stream&>::value_type...>;
        using R = parse_result<Os, E>;

        OptOs opt_results;
        std::optional<err_mode<E>> err;
        
        template for (constexpr auto idx : indices)
        {
            auto& parser = std::get<idx>(m_parsers);
            auto result = parser(stream);

            if (!result.has_value())
            {
                err.emplace(std::move(result.error()));
                break;
            }

            std::get<idx>(opt_results).emplace(std::move(result.value()));
        }

        if (err.has_value())
        {
            return make_recoverable_from_input<R>(stream);
        }

        constexpr auto [...idx] = indices; 

        return R(std::in_place, Os(std::move(std::get<idx>(opt_results).value())...));
    }
};

template <typename... Parsers>
sequence_parser(Parsers&&... ps) -> sequence_parser<std::decay_t<Parsers>...>;

template <typename Parser>
class repeat_parser : public parser_interface
{
    [[no_unique_address]] Parser m_parser;
    occurrences<size_t> m_range;

public:

    static constexpr bool is_always_succeed = false;

    template <typename Parser2>
    constexpr repeat_parser(Parser2&& parser, occurrences<size_t> range)
        : m_parser((Parser2&&) parser), m_range(range) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using O1 = typename std::invoke_result_t<Parser, Stream&>::value_type;
        using O = std::vector<O1>;
        using R = parse_result<O, E>;

        auto collector = O{};

        size_t count = 0;
        auto size = stream.size();

        while (stream.size())
        {
            auto result = m_parser(stream);

            if (!result) 
            {
                if (result.error().is_fatal())
                {
                    return R(std::unexpect, std::move(result.error()));
                }
                break;
            }

            // Both std::vector and std::set support insert with position 
            // collector.insert(collector.end(), std::move(result.value()));
            collector.emplace(collector.end(), std::move(result.value()));
            ++count;

            if (size == stream.size())
            {
                // Current loop will not consume any input, avoid infinite loop
                return make_recoverable_from_input<R>(stream, "`repeat` parsers must always consume");
            }

            if (m_range.is_greater_or_eq_upper(count))
            {
                break;
            }
        }

        if (m_range.is_less_than_lower(count))
        {
            return make_recoverable_from_input<R>(stream);
        }

        return R(std::in_place, std::move(collector));
    }
};

template <typename Parser>
repeat_parser(Parser&&, occurrences<size_t>) -> repeat_parser<std::decay_t<Parser>>;

template <std::integral Integral>
class int_parser : public parser_interface
{
public:

    static constexpr bool is_always_succeed = false;

    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using R = parse_result<Integral, E>;

        int base = 10;

        if (stream.match("0b", true))
        {
            base = 2;
        }
        else if (stream.match("0x", true))
        {
            base = 16;
        }

        Integral result;
        auto [ptr, ec] = std::from_chars(stream.begin(), stream.end(), result, base);

        // Check if parsing was successful
        if (ec == std::errc())
        {
            // Successfully parsed the number
            // Advance stream
            stream.advance(ptr - stream.begin());
            return R(std::in_place, result);
        }

        // Failed to parse the number
        return make_recoverable_from_input<R>(stream);
    }
};

template <std::floating_point Floating>
class float_parser : public parser_interface
{
public:

    static constexpr bool is_always_succeed = false;

    template <typename Stream>
    static constexpr auto operator()(Stream& stream)
    {
        using E = typename Stream::error_type;
        using R = parse_result<Floating, E>;

        Floating result;
        auto [ptr, ec] = std::from_chars(stream.begin(), stream.end(), result);

        // Check if parsing was successful
        if (ec == std::errc())
        {
            // Successfully parsed the number
            // Advance stream
            stream.advance(ptr - stream.begin());
            return R(std::in_place, result);
        }

        // Failed to parse the number
        return make_recoverable_from_input<R>(stream);
    }
};

template <typename CharT>
class till_line_ending_parser : public parser_interface
{
    // Only Linux and MacOS 10+ use LF as newline.
    // MacOS 9 and earlier use CR as newline.
    // The Windows use CRLF as newline.
    static constexpr std::basic_string_view<CharT> line1 = "\n";
    static constexpr std::basic_string_view<CharT> line2 = "\r\n";

public:

    static constexpr bool is_always_succeed = false;

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using O = std::basic_string_view<CharT>;
        using R = parse_result<O, E>;

        auto pos = stream.find_first_of(line2);

        if (pos == line2.npos)
        {
            // EOF reached, return an empty string as the line content.
            return R(std::in_place, stream.advance_and_discard(stream.size()));
        }

        if (stream[pos] == '\r' && stream.peek(pos + 1) != '\n')
        {
            // '\r' must be followed by '\n' to be considered a valid line ending.
            return make_recoverable_from_input<R>(stream);
        }

        return R(std::in_place, stream.advance_and_discard(pos));
    }
};

template <typename Pred>
struct check_next_character_parser : public parser_interface
{
    Pred m_pred;

public:

    static constexpr bool is_always_succeed = false;

    template <typename Pred2>
    constexpr check_next_character_parser(Pred2&& pred) : m_pred((Pred2&&) pred) {}

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using E = typename Stream::error_type;
        using O = std::basic_string_view<typename Stream::value_type>;
        using R = parse_result<O, E>;

        if (stream.size() == 0 || std::invoke(m_pred, stream[0]))
        {
            return make_recoverable_from_input<R>(stream);
        }

        return R(std::in_place, stream.advance_and_discard(1));
    }
};

template <typename Pred>
check_next_character_parser(Pred&&) -> check_next_character_parser<std::decay_t<Pred>>;

struct always_false
{
    template <typename... Ts>
    static constexpr bool operator()(Ts&&...) { return false; }
};


}  // namespace cpp::config::parser::detail


