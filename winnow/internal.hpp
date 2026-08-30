#pragma once

#include "error.hpp"

namespace winnow::detail
{

template <typename Stream>
struct literal_parser
{
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
            return result_type::make_ok(left);
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


}  // namespace winnow::detail
