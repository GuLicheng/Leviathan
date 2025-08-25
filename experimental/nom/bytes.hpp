#pragma once

#include "internal.hpp"

namespace nom::bytes
{

inline constexpr struct
{
    template <typename Prediction>
    static constexpr auto operator()(Prediction&& pred)
    {
        return make_parser_binder(TakeTill<false>(), (Prediction&&)pred);
    }
} take_till;

inline constexpr struct
{
    template <typename Prediction>
    static constexpr auto operator()(Prediction&& pred)
    {
        return make_parser_binder(TakeTill<true>(), (Prediction&&)pred);
    }
} take_till1;

inline constexpr struct
{
    template <typename Normal, typename ControlChar, typename Escapable>
    static constexpr auto operator()(Normal&& n, ControlChar&& c, Escapable&& e)
    {
        return make_parser_binder(Escaped(), (Normal&&)n, (ControlChar&&)c, (Escapable&&)e);
    }
} escaped;

inline constexpr auto take = [](size_t count) static
{
    return [count]<typename ParseContext>(ParseContext& ctx) 
    {
        if (ctx.size() < count)
        {
            return IResult<std::string_view>(
                std::unexpect, 
                std::format("Expected to take {} bytes, but only {} available", count, ctx.size()),
                ErrorKind::Eof
            );
        }

        std::string_view result = { ctx.begin(), count };
        ctx.remove_prefix(count);
        return IResult<std::string_view>(std::in_place, result);
    };
};

inline constexpr struct 
{
    static constexpr auto operator()(const char* input)
    {
        std::string_view sv(input);
        return Tag(sv);
    }

    static constexpr auto operator()(std::string_view input)
    {
        return Tag(input);
    }
} tag;

}   // namespace nom::bytes