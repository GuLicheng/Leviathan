#pragma once

#include "error.hpp"
#include "error_kind.hpp"

namespace nom::detail
{

template <typename Context>
struct tag_parser
{
    using error_type = error<Context, typename Context::error_code>;
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
            return result_type::make_ok(std::move(right), std::move(left));
        }
        else
        {
            return result_type::make_err(std::move(ctx), error<error_kind>::make_recoverable(error_kind::tag));
        }
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




}  // namespace nom::detail

