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
        if (stream.match(value, true))
        {
            auto [left, right] = stream.split_at(value.size());
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


}  // namespace winnow::detail
