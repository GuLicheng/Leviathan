#pragma once

#include "tim_sort.hpp"
#include "common.hpp"

namespace leviathan::ranges
{
    struct tim_sort_fn
    {
        template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
            requires std::sortable<I, Comp, Proj>
        constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
        {
            auto tail = std::ranges::next(first, last);
            tim_sort(first, tail, detail::make_comp_proj(comp, proj));
            return tail;    
        }   

        template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity> 
            requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj> 
        constexpr std::ranges::borrowed_iterator_t<Range> 
        operator()(Range &&r, Comp comp = {}, Proj proj = {}) const 
        {
            return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
        }
    };

    inline constexpr tim_sort_fn tim_sort{};
}
