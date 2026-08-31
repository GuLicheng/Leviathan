
#pragma once

#include "error.hpp"

namespace winnow::detail
{

template <typename Stream>
struct literal_parser 
{
    using stream_type = Stream;
    using literal_type = std::basic_string_view<typename Stream::value_type>;
    using error_type = typename Stream::error_type;
    using result_type = modal_result<literal_type, error_type>;

    literal_type value;

    constexpr literal_parser(literal_type t) : value(t) { }

    constexpr result_type operator()(Stream& stream) const
    {
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

template <typename Stream, typename Pred>
struct take_while_parser
{
    using error_type = typename Stream::error_type;
    using result_type = modal_result<std::basic_string_view<typename Stream::value_type>, error_type>;

    Pred pred;
    size_t min;
    std::optional<size_t> max;

    constexpr take_while_parser(Pred p, size_t min_count, std::optional<size_t> max_count)
        : pred(p), min(min_count), max(max_count) { }


    constexpr result_type operator()(Stream& stream) const
    {
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

template <typename Stream>
struct take_parser
{
    using error_type = typename Stream::error_type;
    using result_type = modal_result<std::basic_string_view<typename Stream::value_type>, error_type>;

    size_t count;

    constexpr take_parser(size_t n) : count(n) { }

    constexpr result_type operator()(Stream& stream) const
    {
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

template <typename Stream>
struct take_until_parser
{
    using literal_type = std::basic_string_view<typename Stream::value_type>;
    using error_type = typename Stream::error_type;
    using result_type = modal_result<literal_type, error_type>;

    literal_type value;
    size_t min;
    std::optional<size_t> max;

    constexpr take_until_parser(literal_type v, size_t min_count, std::optional<size_t> max_count)
        : value(v), min(min_count), max(max_count) { }

    constexpr result_type operator()(Stream& stream) const
    {
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

template <typename Stream, typename IgnoredParser, typename Parser>
struct preceded_parser
{
    using result_type1 = std::invoke_result_t<IgnoredParser, Stream&>;
    using result_type2 = std::invoke_result_t<Parser, Stream&>;    
    
    using error_type1 = typename result_type1::error_type;
    using error_type2 = typename result_type2::error_type;

    using error_type = typename Stream::error_type;
    using result_type = result_type2;

    IgnoredParser ignored_parser;
    Parser parser;

    constexpr preceded_parser(IgnoredParser ip, Parser p) 
        : ignored_parser(std::move(ip)), parser(std::move(p)) { }

    constexpr result_type operator()(Stream& stream) const
    {
        auto ignored_result = ignored_parser(stream);

        if (!ignored_result)
        {
            return result_type::make_err(std::move(ignored_result.unwrap_err()));
        }
        return parser(stream);
    }

};

template <typename Stream, typename Parser, typename IgnoredParser>
struct terminated_parser
{
    using result_type1 = std::invoke_result_t<Parser, Stream&>;
    using result_type2 = std::invoke_result_t<IgnoredParser, Stream&>;    
    
    using error_type1 = typename result_type1::error_type;
    using error_type2 = typename result_type2::error_type;

    using error_type = typename Stream::error_type;
    using result_type = result_type1;

    Parser parser;
    IgnoredParser ignored_parser;

    constexpr terminated_parser(Parser p, IgnoredParser ip) 
        : parser(std::move(p)), ignored_parser(std::move(ip)) { }

    constexpr result_type operator()(Stream& stream) const
    {
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

template <typename Stream, typename Parser1, typename SepParser, typename Parser2>
struct separated_pair_parser
{
    using result_type1 = std::invoke_result_t<Parser1, Stream&>;
    using result_type2 = std::invoke_result_t<Parser2, Stream&>;

    using error_type = typename Stream::error_type;
    using output_type = std::pair<typename result_type1::value_type, typename result_type2::value_type>;
    using result_type = modal_result<output_type, error_type>;

    Parser1 parser1;
    SepParser sep_parser;
    Parser2 parser2;

    constexpr separated_pair_parser(Parser1 p1, SepParser sp, Parser2 p2) 
        : parser1(std::move(p1)), sep_parser(std::move(sp)), parser2(std::move(p2)) { }

    constexpr result_type operator()(Stream& stream) const
    {
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

template <typename Stream, typename Parser, typename F>
struct map_parser
{
    using result_type1 = std::invoke_result_t<Parser, Stream&>;
    using error_type = typename Stream::error_type;
    using output_type1 = typename result_type1::value_type;
    using output_type = std::invoke_result_t<F, output_type1>;
    using result_type = modal_result<output_type, error_type>;

    Parser parser;
    F func;

    constexpr map_parser(Parser p, F f) : parser(std::move(p)), func(std::move(f)) { }

    constexpr result_type operator()(Stream& stream) const
    {
        auto result = parser(stream);

        if (!result)
        {
            return result_type::make_err(std::move(result.unwrap_err()));
        }
        return result_type::make_ok(std::invoke(func, std::move(result.unwrap_ok())));
    }
};

template <typename Stream, typename Parser, typename P>
struct verify_parser
{
    using error_type = typename Stream::error_type;
    using result_type = std::invoke_result_t<Parser, Stream&>;

    Parser parser;
    P predicate;

    constexpr verify_parser(Parser p, P pred) : parser(std::move(p)), predicate(std::move(pred)) { }

    constexpr result_type operator()(Stream& stream) const
    {
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

}  // namespace winnow::detail


