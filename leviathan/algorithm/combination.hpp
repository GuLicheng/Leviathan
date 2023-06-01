#pragma once

#include <algorithm>
#include <functional>

namespace leviathan::detail
{
    template <typename I>
    constexpr void shift_left(I first1, I last1, I first2, I last2, std::bidirectional_iterator_tag)
    {
        if (first1 == last1 || first2 == last2)
            return;

        std::reverse(first1, last1);
        std::reverse(first2, last2);

        while (first1 != last1 && first2 != last2)
        {
            std::iter_swap(first1, --last2);
            ++first1;
        }

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
    template <typename I>
    constexpr void shift_left(I first1, I last1, I first2, I last2, std::random_access_iterator_tag)
    {
        if (first1 == last1 || first2 == last2)
            return;

        const auto left_size = last1 - first1;
        const auto right_size = last2 - first2;

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
            return false;

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
        shift_left(left, middle, right, last, typename std::iterator_traits<I>::iterator_category());

        // Return false to stop do-while loop.
        return !is_over;
    }
} // namespace detail

namespace leviathan
{
    template <typename I, typename Comp = std::less<>>
    constexpr bool next_combination(I first, I middle, I last, Comp comp = {})
    {
        return detail::combination_impl(first, middle, last, comp);
    }

    template <typename I, typename Comp = std::greater<>>
    constexpr bool prev_combination(I first, I middle, I last, Comp comp = {})
    {
        return detail::combination_impl(first, middle, last, comp);
    }
} // namespace leviathan
