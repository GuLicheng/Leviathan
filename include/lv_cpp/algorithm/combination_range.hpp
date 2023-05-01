#pragma once

#include "common.hpp"
#include "combination.hpp"

namespace leviathan::ranges
{
    template<typename Iter>
    using next_combination_result = std::ranges::in_found_result<Iter>;

    struct next_combination_fn
    {
        template <std::bidirectional_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
        constexpr next_combination_result<I> operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) const
        {
            auto tail = std::ranges::next(first, last);
            return { tail, detail::combination_impl(first, middle, tail, detail::make_comp_proj(comp, proj)) };
        } 

        template <std::ranges::bidirectional_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
        constexpr next_combination_result<std::ranges::borrowed_iterator_t<R>>
        operator()(R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {}) const
        {
            return (*this)(std::ranges::begin(r), std::move(middle), std::ranges::end(r),
                    std::move(comp), std::move(proj));
        }
    };

    inline constexpr next_combination_fn next_combination{};

    template<typename Iter>
    using prev_combination_result = std::ranges::in_found_result<Iter>;

    struct prev_combination_fn
    {
        template <std::bidirectional_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::greater, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
        constexpr prev_combination_result<I> operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) const
        {
            auto tail = std::ranges::next(first, last);
            return { tail, detail::combination_impl(first, tail, detail::make_comp_proj(comp, proj)) };
        } 

        template <std::ranges::bidirectional_range R, typename Comp = std::ranges::greater, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
        constexpr prev_combination_result<std::ranges::borrowed_iterator_t<R>>
        operator()(R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {}) const
        {
            return (*this)(std::ranges::begin(r), std::move(middle), std::ranges::end(r),
                    std::move(comp), std::move(proj));
        }
    };

    inline constexpr prev_combination_fn prev_combination{};
}
