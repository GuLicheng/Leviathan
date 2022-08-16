#pragma once

#include <algorithm>
#include "common.hpp"

namespace leviathan::detail
{
    template <typename I, typename Comp>
    constexpr bool combination(I first1, I middle, I last2, Comp comp)
    {

        auto ori_f = first1, ori_l = last2;

        auto last1 = middle, first2 = middle;

        if (first1 == last1 || first2 == last2)
            return false;

        I m1 = last1;
        I m2 = last2;
        --m2;

        // find first element less than last element
        while (--m1 != first1 && !comp(*m1, *m2))
            ;

        bool result = m1 == first1 && !comp(*first1, *m2);
        if (!result)
        {
            while (first2 != m2 && !comp(*m1, *first2))
                ++first2;

            first1 = m1;
            std::iter_swap(first1, first2);
            ++first1;
            ++first2;
        }

        if (first1 != last1 && first2 != last2)
        {
            m1 = last1;
            m2 = first2;

            while (m1 != first1 && m2 != last2)
            {
                std::iter_swap(--m1, m2);
                ++m2;
            }

            std::reverse(first1, m1);
            std::reverse(first1, last1);
            std::reverse(m2, last2);
            std::reverse(first2, last2);
        }

        return !result;
    }

} // namespace detail

namespace leviathan
{

    // template <typename I>
    // constexpr bool next_combination(I first, I middle, I last)
    // {
    //     return combination(first, middle, last, std::less<>());
    // }

    // template <typename I>
    // constexpr bool prev_combination(I first, I middle, I last)
    // {
    //     return combination(first, middle, last, std::greater<>());
    // }


    template<typename Iter>
    using next_combination_result = std::ranges::in_found_result<Iter>;


    /*
        std::vector values = { 1, 2, 3 };
        do 
        {
            for (auto val : values)
                std::cout << val << ' ';
            std::endl(std::cout);
        } while (leviathan::next_combination(values, values.begin() + 2).found);
        
        Output:
            1 2 3
            1 3 2
            2 3 1

        std::vector values = { 1, 2, 2 };
        do 
        {
            for (auto val : values)
                std::cout << val << ' ';
            std::endl(std::cout);
        } while (leviathan::next_combination(values, values.begin() + 2).found);
        
        Output:
            1 2 2
            2 2 1
    */
    struct next_combination_fn
    {
        template <std::bidirectional_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
        constexpr next_combination_result<I> operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) const
        {
            auto tail = std::ranges::next(first, last);
            return { std::move(tail), detail::combination(first, middle, tail, detail::make_comp_proj(comp, proj)) };
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
            return { std::move(tail), detail::combination(first, tail, detail::make_comp_proj(comp, proj)) };
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
