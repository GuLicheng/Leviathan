#pragma once

#include "internal.hpp"

namespace nom::bytes
{

/*=========================== take_till ===========================*/
inline constexpr auto take_till = []<typename Pred>(Pred&& pred) static
{
    return ConditionalLoop0(std::not_fn((Pred&&)pred));
};

inline constexpr auto take_till1 = []<typename Pred>(Pred&& pred) static
{
    return ConditionalLoop1(std::not_fn((Pred&&)pred), ErrorKind::TakeTill1);
};
/*=========================== take_till ===========================*/

/*=========================== take_while ===========================*/
inline constexpr auto take_while0 = []<typename Pred>(Pred&& pred) static
{
    return ConditionalLoop0<std::decay_t<Pred>>((Pred&&)pred);
};

inline constexpr auto take_while1 = []<typename Pred>(Pred&& pred) static
{
    return ConditionalLoop1<std::decay_t<Pred>>((Pred&&)pred, ErrorKind::TakeWhile1);
};
/*=========================== take_while ===========================*/

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
    static constexpr auto operator()(std::string_view input)
    {
        return Tag(input);
    }

    static constexpr auto operator()(const char* input)
    {
        std::string_view sv(input);
        return operator()(sv);
    }
} tag;

inline constexpr auto is_a = [](std::string_view input) static
{
    auto searcher = [=]<typename CharT>(CharT ch) { return input.contains(ch); };
    return ConditionalLoop1<std::decay_t<decltype(searcher)>>(searcher, ErrorKind::IsA);
};

inline constexpr auto is_not = [](std::string_view input) static
{
    auto searcher = [=]<typename CharT>(CharT ch) { return !input.contains(ch); };
    return ConditionalLoop1<std::decay_t<decltype(searcher)>>(searcher, ErrorKind::IsNot);
};

}   // namespace nom::bytes