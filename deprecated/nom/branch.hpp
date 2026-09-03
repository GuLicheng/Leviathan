#pragma once

#include "internal.hpp"

// https://docs.rs/nom/latest/nom/branch/index.html
namespace nom::branch
{

inline constexpr auto alt = []<typename... Fns>(Fns... fns) static
{
    return [...fns = std::move(fns)]<typename Context>(Context ctx)
    {
        return detail::choice<Context, Fns...>(std::move(fns)...)(std::move(ctx));
    };
}; 

// TODO: implement 'permutation'

}  // namespace nom::branch