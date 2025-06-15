/*
    https://webpages.charlotte.edu/rbunescu/courses/ou/cs4040/introsort.pdf
    https://en.wikipedia.org/wiki/Introsort

    Pseudocode:
        procedure sort(A : array):
        maxdepth ← ⌊log2(length(A))⌋ × 2
        introsort(A, maxdepth)

    procedure introsort(A, maxdepth):
        n ← length(A)
        if n < 16:
           insertionsort(A)
        else if maxdepth = 0:
            heapsort(A)
        else:
            p ← partition(A)  // assume this function does pivot selection, p is the final position of the pivot
            introsort(A[1:p-1], maxdepth - 1)
            introsort(A[p+1:n], maxdepth - 1)
*/

#pragma once

#include <cmath>
#include <utility>

#include "basic_sort.hpp"

namespace cpp::ranges::detail
{

enum class pivot_mode
{
    median3,
};

template <int InsertionSortThreshold = 24, pivot_mode Mode = pivot_mode::median3> 
class intro_sorter 
{
protected:

    template <typename I, typename Comp>
    static constexpr void sort3(I first, I middle, I last, Comp comp)
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
        --last;
        auto middle = first + ((last - first) >> 1);

        sort3(first, middle, last, comp);
        --last;
        std::iter_swap(middle, last);
        return last;
    }

    template <typename I, typename Comp>
        requires (Mode == pivot_mode::median3)
    static constexpr I partition_pivot(I first, I last, Comp comp)
    {
        const auto pivot = median_three(first, last, comp);
        auto i = first, j = pivot;

        // median_three keep first <= pivot <= last
        // [first, ..., pivot, last]
        // so the first and last can be used as sentinels to prevent going out of bounds.
        while (1) 
        {
            while (comp(*(++i), *pivot));
            while (comp(*pivot, *(--j)));
            if (i < j) std::iter_swap(i, j);
            else break;
        }

        std::iter_swap(i, pivot);
        return i;
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
        
        auto pivot = partition_pivot(first, last, comp);
        intro_sort_recursive(first, pivot, comp, depth - 1);
        intro_sort_recursive(pivot + 1, last, comp, depth - 1);
    }

public:

    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {
        using DifferenceType = typename std::iterator_traits<I>::difference_type;
        const DifferenceType n = std::distance(first, last);
        const auto max_depth = std::countl_zero(std::make_unsigned_t<DifferenceType>(n)) << 1; // 2 * log2(n)
        intro_sort_recursive(first, last, comp, max_depth);
    }
};

}  // namespace cpp::ranges::detail

namespace cpp::ranges
{

inline constexpr detail::sorter<detail::intro_sorter<>> intro_sort; 

}
