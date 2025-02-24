#pragma once

#include <type_traits>
#include <functional>
#include <iterator>
#include <algorithm>

namespace leviathan::algorithm::detail
{

template <typename T>
inline constexpr bool pass_by_value_v = std::is_class_v<T> && std::is_empty_v<T>;

template <typename Comp, typename Proj>
constexpr auto make_comp_proj(Comp& comp, Proj& proj)
{
    if constexpr (pass_by_value_v<Comp> && pass_by_value_v<Proj>)
    {
        return [=]<typename L, typename R>(L&& lhs, R&& rhs) -> bool {
            return std::invoke(comp, 
                std::invoke(proj, (L&&)lhs),
                std::invoke(proj, (R&&)rhs)
            );
        };
    }
    else    
    {
        return [&]<typename L, typename R>(L&& lhs, R&& rhs) -> bool {
            return std::invoke(comp, 
                std::invoke(proj, (L&&)lhs),
                std::invoke(proj, (R&&)rhs)
            );
        };
    }
}

template <typename Callable>
auto ref_or_value(Callable& callable)
{
    if constexpr (pass_by_value_v<Callable>)
    {
        return callable;
    }
    else
    {
        return std::ref(callable);
    }
}

} // namespace leviathan::algorithm::detail

// TODO: Add range-version
#if 0
namespace leviathan::algorithm
{
    template <typename I, typename Comp = std::less<>>
    std::pair<I, I> min_and_second_min_element(I first, I last, Comp comp = {})
    {
        auto forward = std::ranges::next(first, 1, last);
        std::pair<I, I> result { first, forward };

        if (forward == last)
            return result;

        if (!comp(result.first, result.second))
            std::swap(result.first, result.second);
        
        for (; forward != last; ++forward) 
        {
            // result.first < result.second 
            if (comp(*forward, *result.first))
            {
                result.second = result.first;
                result.first = forward;
            }
            else if (comp(*forward, *result.second))
                result.second = forward;
        }

        return result;
    }
}
#endif