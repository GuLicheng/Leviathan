#pragma once

#include <leviathan/extc++/expected.hpp>

#include "error.hpp"

namespace winnow
{

template <typename O, typename E>
using modal_result = std::expected<O, err_mode<E>>;

template <typename R, typename Stream>
constexpr auto make_backtrack_from_input(Stream& stream, const char* message = nullptr)
{
    // R is modal_result
    using ErrMode = typename R::error_type;
    using O = typename R::value_type;
    using E = typename ErrMode::error_type;
    return modal_result<O, E>(
        std::unexpect, ErrMode::make_backtrack(error_traits<E>::from_input(stream, message))
    );
}


}  // namespace winnow

template <typename O, typename E>
struct std::formatter<winnow::modal_result<O, E>> 
{
    template <typename FormatContext>
    static constexpr auto parse(FormatContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    static constexpr auto format(const winnow::modal_result<O, E>& r, FormatContext& ctx) 
    {
        if (r.has_value())
        {
            return std::format_to(ctx.out(), "Ok({:n})", r.value());
        }
        else
        {
            return std::format_to(ctx.out(), "Err({})", r.error());
        }
    }
};

