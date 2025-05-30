#pragma once

#include <assert.h>
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

template <typename Sorter>
struct sorter
{
    template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto tail = std::ranges::next(first, last);
        Sorter()(first, tail, detail::make_comp_proj(comp, proj));
        return tail;    
    }   

    template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity> 
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj> 
    constexpr std::ranges::borrowed_iterator_t<Range> static operator()(Range &&r, Comp comp = {}, Proj proj = {})  
    {
        return operator()(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }
};

} // namespace cpp::algorithm::detail

