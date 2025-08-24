#pragma once

#include "error.hpp"

namespace nom
{
    
template <typename T, typename F>
class Value
{
    T value;
    F parser;

public:

    constexpr Value(T v, F f) : value(std::move(v)), parser(std::move(f)) { }

    template <typename ParseContext>
    constexpr IResult<T> operator()(ParseContext& ctx)
    {
        auto result = parser(ctx); 
        
        if (result)
        {
            return std::move(value);
        }
        else
        {
            return IResult<T>(std::unexpect, std::move(result.error()));
        }
    }
};

inline constexpr struct 
{
    template <typename T, typename F>
    static constexpr auto operator()(T v, F f)
    {
        return Value<T, F>(std::move(v), std::move(f));
    }
} value;

} // namespace nom

