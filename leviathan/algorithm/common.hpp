#pragma once

#include <assert.h>
#include <type_traits>
#include <functional>
#include <iterator>
#include <algorithm>
#include <new>

namespace cpp::ranges::detail
{

consteval auto L1cacheline()
{
    return std::hardware_destructive_interference_size;
}

consteval auto L2cacheline()
{
    return std::hardware_constructive_interference_size;
}

template <typename Comp, typename Proj>
constexpr auto make_comp_proj(Comp& comp, Proj& proj)
{
    return [&]<typename L, typename R>(L&& lhs, R&& rhs) -> bool {
        return std::invoke(comp, 
            std::invoke(proj, (L&&)lhs),
            std::invoke(proj, (R&&)rhs)
        );
    };
}

template <typename Pred, typename Proj>
constexpr auto make_pred_proj(Pred& pred, Proj& proj)
{
    return [&]<typename T>(T&& value) -> bool {
        return std::invoke(pred, std::invoke(proj, (T&&)value));
    };
}

} // namespace cpp::algorithm::detail

