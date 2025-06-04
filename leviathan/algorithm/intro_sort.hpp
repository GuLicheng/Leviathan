// https://webpages.charlotte.edu/rbunescu/courses/ou/cs4040/introsort.pdf

#pragma once

#include <cmath>
#include <utility>

#include "basic_sort.hpp"

namespace cpp::ranges::detail
{

enum class pivot_mode
{
    median_three,
};

template <int InsertionSortThreshold = 16, pivot_mode Mode = pivot_mode::median_three> 
class intro_sorter 
{
protected:

    template <typename I, typename Comp>
    static constexpr void heap_sort(I first, I last, Comp comp)
    {
        std::make_heap(first, last, comp);
        std::sort_heap(first, last, comp);
    }

    template <typename I, typename Comp>
    static constexpr void sort_three(I first, I middle, I last, Comp comp)
    {
        if (comp(*middle, *first)) 
        {
            std::iter_swap(middle, first);
        }

        if (comp(*last, *middle)) 
        {
            std::iter_swap(last, middle);
            if (comp(*middle, *first))
            {
                std::iter_swap(middle, first);
            }
        }
    }

    template <typename I, typename Comp>
    static constexpr I median_three(I first, I last, Comp comp)
    {
        // [first, middle, last]
        // keep first < middle < last
        auto middle = first + ((last - first) >> 1);

        sort_three(first, middle, last, comp);
        --last;
        std::iter_swap(middle, last);
        return last;
    }

    template <typename I, typename Comp>
    static constexpr I select_pivot(I first, I last, Comp comp)
    {
        if constexpr (Mode == pivot_mode::median_three)
        {
            return median_three(first, last, comp);
        }
        std::unreachable();
    }

    template <typename I, typename Comp>
    static constexpr void intro_sort_recursive(I first, I last, Comp comp, int depth)
    {
        if (last - first < InsertionSortThreshold) 
        {
            insertion_sort(first, last, comp);
            return;
        }

        if (depth == 0) 
        {
            heap_sort(first, last, comp);
            return;
        }
        
        const auto pivot = select_pivot(first, last - 1, comp);
        auto i = first, j = pivot;

        while (1) 
        {
            while (comp(*(++i), *pivot));
            while (comp(*pivot, *(--j)));
            if (i < j) std::iter_swap(i, j);
            else break;
        }

        std::iter_swap(i, pivot);
        intro_sort_recursive(first, i, comp, depth - 1);
        intro_sort_recursive(i + 1, last, comp, depth - 1);
    }

    template <typename I, typename Comp>
    static constexpr void intro_sort_recursive_with_leftmost(I first, I last, Comp comp, int depth, bool leftmost = true)
    {
        if (last - first < InsertionSortThreshold) 
        {
            leftmost ? insertion_sort(first, last, comp) : unguarded_insertion_sort(first, last, comp);
            return;
        }

        if (depth == 0) 
        {
            heap_sort(first, last, comp);
            return;
        }
        
        const auto pivot = select_pivot(first, last - 1, comp);
        auto i = first, j = pivot;

        while (1) 
        {
            while (comp(*(++i), *pivot));
            while (comp(*pivot, *(--j)));
            if (i < j) std::iter_swap(i, j);
            else break;
        }

        std::iter_swap(i, pivot);
        intro_sort_recursive_with_leftmost(first, i, comp, depth - 1, leftmost);
        intro_sort_recursive_with_leftmost(i + 1, last, comp, depth - 1, false);
    }

public:

    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {
        using DifferenceType = typename std::iterator_traits<I>::difference_type;
        DifferenceType n = std::distance(first, last);
        const auto max_depth = std::countl_zero(std::make_unsigned_t<DifferenceType>(n)) << 1; // 2 * log2(n)
        intro_sort_recursive(first, last, comp, max_depth);
    }

};

}  // namespace cpp::ranges::detail

namespace cpp::ranges
{

inline constexpr detail::sorter<detail::intro_sorter<>> intro_sort; 

}
