#pragma once

#include <type_traits>
#include <functional>
#include <iterator>
#include <algorithm>

namespace cpp::ranges::detail
{

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

// TODO: Add range-version
#if 0
namespace cpp::algorithm
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
                // result.second = std::exchange(result.first, forward);
            }
            else if (comp(*forward, *result.second))
            {
                result.second = forward;
            }
        }

        return result;
    }
}
#endif