
#pragma once

#include "error.hpp"

namespace winnow::detail
{

template <typename Parser, typename F> struct map_parser;

template <typename Parser, typename P> struct verify_parser;

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
};

template <typename Parser, typename P>
struct verify_parser
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














template <typename CharT>
struct literal_parser : parser_interface
{
    using literal_type = std::basic_string_view<CharT>;

    literal_type value;

    constexpr literal_parser(literal_type t) : value(t) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<literal_type, error_type>;

        if (stream.match(value, false))
        {
            auto [left, right] = stream.split_at(value.size());
            stream = right;
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
    size_t min;
    std::optional<size_t> max;

    constexpr take_while_parser(Pred p, size_t min_count, std::optional<size_t> max_count)
        : pred(p), min(min_count), max(max_count) { }
    
    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<std::basic_string_view<typename Stream::value_type>, error_type>;
        
        // User should ensure that the max is not less than min.
        size_t count = 0;

        while (count < stream.size() && std::invoke(pred, stream[count]))
        {
            if (max && count >= *max)
            {
                break;
            }
            ++count;
        }

        if (count < min)
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
            stream = right;
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
            stream = right;
            return result_type::make_ok(std::move(left));
        }
    }
};

template <typename CharT>
struct take_until_parser : parser_interface
{
    using literal_type = std::basic_string_view<CharT>;
    
    literal_type value;
    size_t min;
    std::optional<size_t> max;

    constexpr take_until_parser(literal_type v, size_t min_count, std::optional<size_t> max_count)
        : value(v), min(min_count), max(max_count) { }

    template <typename Stream>
    constexpr auto operator()(Stream& stream) const
    {
        using error_type = typename Stream::error_type;
        using result_type = modal_result<literal_type, error_type>;
        
        const auto left = min;
        const auto right = max.value_or(stream.size());
        const auto slice = stream.to_string_view().substr(left, right);

        const auto pos = slice.find(value);

        if (pos == literal_type::npos)
        {   
            return result_type::make_err(
                err_mode<error_type>::make_backtrack(
                    error_traits<error_type>::from_input(stream)
                )
            );
        }
        auto [left_part, right_part] = stream.split_at(left + pos);
        stream = right_part;
        return result_type::make_ok(std::move(left_part));
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
        stream = right;
        return result_type::make_ok(std::move(left));
    }
};


}  // namespace winnow::detail


