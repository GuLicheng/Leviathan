#pragma once

namespace nom::detail
{

template <typename Context, typename Error>
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


}  // namespace nom::detail

