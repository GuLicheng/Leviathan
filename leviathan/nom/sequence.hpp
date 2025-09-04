/*
    We only implement a very small subset of nom with nom::complete part.
    The streaming part is not implemented.

    TODO:

    - all sequence components in have been implemented.
    
*/
#pragma once

#include "error.hpp"
#include "internal.hpp" 

// https://docs.rs/nom/latest/nom/sequence/index.html
namespace nom::sequence
{

inline constexpr auto pair = []<typename F1, typename F2>(F1 f1, F2 f2) static
{
    return [f1 = std::move(f1), f2 = std::move(f2)]<typename Context>(Context ctx) 
    {
        return detail::pair_fn<Context, F1, F2>(std::move(f1), std::move(f2))(std::move(ctx));
    };
};

inline constexpr auto preceded = []<typename F1, typename F2>(F1 f1, F2 f2) static
{
    return [f1 = std::move(f1), f2 = std::move(f2)]<typename Context>(Context ctx) 
    {
        return detail::preceded_fn<Context, F1, F2>(std::move(f1), std::move(f2))(std::move(ctx));
    };
};

inline constexpr auto terminated = []<typename F1, typename F2>(F1 f1, F2 f2) static
{
    return [f1 = std::move(f1), f2 = std::move(f2)]<typename Context>(Context ctx) 
    {
        return detail::terminated_fn<Context, F1, F2>(std::move(f1), std::move(f2))(std::move(ctx));
    };
};
 
inline constexpr auto delimited = []<typename F1, typename F2, typename F3>(F1 f1, F2 f2, F3 f3) static
{
    return [f1 = std::move(f1), f2 = std::move(f2), f3 = std::move(f3)]<typename Context>(Context ctx) 
    {
        return preceded(std::move(f1), terminated(std::move(f2), std::move(f3)))(std::move(ctx));
    };
};

inline constexpr auto separated_pair = []<typename F1, typename Sep, typename F2>(F1 f1, Sep sep, F2 f2) static
{
    return pair(std::move(f1), preceded(std::move(sep), std::move(f2)));
};

}  // namespace nom::sequence