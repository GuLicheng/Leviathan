#pragma once

#include "error.hpp"
#include "error_kind.hpp"
#include "result.hpp"

namespace nom::detail
{

template <typename Context>
class tag_parser
{

    using ErrorCode = typename Context::error_code;

public:

    using char_type = typename Context::value_type;
    using tag_type = std::basic_string_view<char_type>;
    using output_type = Context;
    using error_type = error<Context, ErrorCode>;
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
            return result_type::make_err(
                err<error_type>::make_recoverable(
                    std::move(ctx), 
                    error_traits<ErrorCode>::from_error_kind(error_kind::tag)
                )
            );
        }
    }
};

template <typename Context, typename Prediction, bool RequireAtLeastOne>
class loop_parser
{
    using ErrorCode = typename Context::error_code;

public:

    using error_type = error<Context, ErrorCode>;
    using result_type = iresult<Context, Context, error_type>;

    Prediction pred;

    constexpr loop_parser(Prediction p) : pred(std::move(p)) { }

    constexpr result_type operator()(Context ctx) requires(RequireAtLeastOne)
    {
        auto result = parse(ctx);

        if (!result || result->second.empty())
        {
            // return result_type(rust::unexpect, std::move(ctx), code);
            return result_type::make_err(
                err<error_type>::make_recoverable(
                    std::move(ctx), 
                    error_traits<ErrorCode>::from_error_kind(error_kind::take_while1)
                )
            );
        }

        return result;
    }

    constexpr result_type operator()(Context ctx)
    {
        return parse(std::move(ctx));
    }

private:

    constexpr result_type parse(Context ctx)
    {
        auto first = ctx.begin(), last = ctx.end();

        for (; first != last && std::invoke(pred, *first); ++first);

        auto [left, right] = ctx.split_at(std::distance(ctx.begin(), first));
        return result_type::make_ok(std::move(right), std::move(left));
    }

};


}  // namespace nom::detail

