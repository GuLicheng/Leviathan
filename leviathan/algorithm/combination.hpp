// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2639.pdf

#pragma once

#include "common.hpp"

namespace cpp::ranges::detail
{

template <std::bidirectional_iterator I>
constexpr void shift_left(I first1, I last1, I first2, I last2)
{
    if (first1 == last1 || first2 == last2)
    {
        return;
    }
    
    // swap_from_first
    //    [1, 2][3, 4, 5]
    // => [2, 1][5, 4, 3]
    std::reverse(first1, last1);
    std::reverse(first2, last2);

    // => [3, 4][5, 1, 2]
    while (first1 != last1 && first2 != last2)
    {
        std::iter_swap(first1++, --last2);
    }

    // => [3, 4][5, 1, 2]
    if (first1 == last1)
    {
        std::reverse(first2, last2);
    }
    else
    {
        std::reverse(first1, last1);
    }
}

/*
    shift elements between two ranges
    [a, b, c][d, e] -> [d, e, a][b, c]
*/
template <std::random_access_iterator I>
constexpr void shift_left(I first1, I last1, I first2, I last2)
{
    if (first1 == last1 || first2 == last2)
    {
        return;
    }

    const auto left_size = std::distance(first1, last1);
    const auto right_size = std::distance(first2, last2);

    if (left_size < right_size)
    {
        // swap_from_first
        //    [1, 2][3, 4, 5]
        // => [3, 4][1, 2, 5] 
        // => [3, 4][5, 1, 2] 
        auto middle = std::swap_ranges(first1, last1, first2);
        std::rotate(first2, middle, last2);
    }
    else
    {
        // swap_from_last
        //    [1, 2, 3][4, 5]
        // => [1, 4, 5][2, 3] 
        // => [4, 5, 1][2, 3] 
        auto middle = first1 + left_size - right_size;
        std::swap_ranges(first2, last2, middle);
        std::rotate(first1, middle, last1);
    }
}

template <typename I, typename Comp>
constexpr bool combination_impl(I first, I middle, I last, Comp comp)
{
    if (first == middle || middle == last)
    {
        return false;
    }

    auto left = middle, right = last;

    --right;
    --left;

    // The left should less than right.
    for (; left != first && !comp(*left, *right); --left);

    // If all elements in left is greater than right, the iteration should be stopped.
    bool is_over = left == first && !comp(*first, *right);

    right = middle;
    
    if (!is_over)
    {
        // Find a_k + 1
        for (; right != last && !comp(*left, *right); ++right);
        std::iter_swap(left++, right++);
    }

    // Replace (a_1, ..., a_{k-1}) with (a_k + 1, ..., a_k + r - k + 1)
    shift_left(left, middle, right, last);

    // Return false to stop do-while loop.
    return !is_over;
}

} // namespace detail

namespace cpp::ranges
{

template <typename Iter>
using next_combination_result = std::ranges::in_found_result<Iter>;

inline constexpr struct 
{
    template <std::bidirectional_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr next_combination_result<I> operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto tail = std::ranges::next(first, last);
        return { tail, detail::combination_impl(first, middle, tail, detail::make_comp_proj(comp, proj)) };
    } 

    template <std::ranges::bidirectional_range R, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
    static constexpr next_combination_result<std::ranges::borrowed_iterator_t<R>> operator()(R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {}) 
    {
        return operator()(std::ranges::begin(r), std::move(middle), std::ranges::end(r),
                std::move(comp), std::move(proj));
    }
} next_combination;

template<typename Iter>
using prev_combination_result = std::ranges::in_found_result<Iter>;

inline constexpr struct 
{
    template <std::bidirectional_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::greater, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr prev_combination_result<I> operator()(I first, I middle, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto tail = std::ranges::next(first, last);
        return { tail, detail::combination_impl(first, tail, detail::make_comp_proj(comp, proj)) };
    } 

    template <std::ranges::bidirectional_range R, typename Comp = std::ranges::greater, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<R>, Comp, Proj>
    static constexpr prev_combination_result<std::ranges::borrowed_iterator_t<R>> operator()(R&& r, std::ranges::iterator_t<R> middle, Comp comp = {}, Proj proj = {}) 
    {
        return operator()(std::ranges::begin(r), std::move(middle), std::ranges::end(r),
                std::move(comp), std::move(proj));
    }
} prev_combination;

}
