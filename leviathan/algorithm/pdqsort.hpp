/*
    
    https://arxiv.org/pdf/2106.05123

    int* part_left(int* l, int* r) {
        int* i = l; int* j = r;
        int p = *l;

        while (*--j > p);
        if (j + 1 == r) {
            while (i < j && *++i <= p);
        } else {
            while (*++i <= p);
        }   

        while (i < j) {
            std::swap(*i, *j);
            while (*--j > p);
            while (*++i <= p);
        }

        std::swap(*l, *j);
        return j;
    }

*/


#pragma once

#include "basic_sort.hpp"

namespace cpp::ranges::detail
{

template <
    int InsertionSortThreshold = 24, 
    int NintherThreshold = 128,
    int PartialInsertionSortLimit = 8,
    int BlockSize = 64,
    int CachelineSize = std::hardware_destructive_interference_size,
    bool Branchless = false> 
class pdq_sorter
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
    static constexpr void partition_insertion_sort(I first, I last, Comp comp);

    // We assume that p was selected as the median of at least three elements,
    // and in later iterations previous elements are used as sentinels to prevent going out of
    // bounds
    template <typename I, typename Comp>
    static constexpr I partition_left(I first, I last, Comp comp)
    {
        I i = first, j = last;
        I pivot = first;

        // Find the first element not less than the pivot
        while (comp(*pivot, *--j));

        if (j + 1 == last)
        {
            while (i < j && !comp(*pivot, *++i));
        }
        else
        {
            while (!comp(*pivot, *++i));
        }

        while (i < j) 
        {
            std::iter_swap(i, j);
            while (comp(*pivot, *--j));
            while (!comp(*pivot, *++i));
        }

        std::iter_swap(j, pivot);
        return j;
    }

    template <typename I, typename Comp>
        requires (!Branchless)
    static constexpr std::pair<I, bool> partition_right(I first, I last, Comp comp)
    {
        I i = first, j = last;
        I pivot = first;

        while (comp(*++i, *pivot));
    
        if (i - 1 == first)
        {
            while (i < j && !comp(*--j, *pivot));
        }
        else
        {
            while (!comp(*--j, *pivot));
        }

        bool no_swaps = i >= j;

        while (i < j) 
        {
            std::iter_swap(i, j);
            while (comp(*++i, *pivot));
            while (!comp(*--j, *pivot));
        }

        std::iter_swap(pivot, i - 1);
        return std::make_pair(i - 1, no_swaps);
    }

    template <typename I, typename Comp>
        requires (Branchless)
    static constexpr std::pair<I, bool> partition_right(I first, I last, Comp comp)
    {
        I i = first, j = last;
        I pivot = first;

        while (comp(*++i, *pivot));
    
        if (i - 1 == first)
        {
            while (i < j && !comp(*--j, *pivot));
        }
        else
        {
            while (!comp(*--j, *pivot));
        }

        bool no_swaps = i >= j;

        if (!no_swaps)
        {
            std::iter_swap(i, j);
            ++i;

            constexpr auto offsets_size = BlockSize + CachelineSize;

            alignas(CachelineSize) unsigned char offsets_l[offsets_size];
            alignas(CachelineSize) unsigned char offsets_r[offsets_size];

            // I offsets_l_base = first;
            // I offsets_r_base = last;

            int num_l = 0, num_r = 0;

            while (first < last)
            {
                // [1, 2, 3, ..., 3, 4, 2]
                const auto rest = std::distance(first, last);

                // 1. Both num_l and num_r are zero: split the block into two halves.
                // 2. Only num_l is zero: 


                // Store offsets for left and right partitions
                for (int i = 0; i < BlockSize; ++i)
                {
                    offsets_l[num_l] = i;
                    num_l += !comp(*(first + i), *pivot);
                }

                for (int i = 0; i < BlockSize; ++i)
                {
                    offsets_r[num_r] = i + 1;
                    num_r += comp(*(last - 1 - i), *pivot);
                }

                // Swap elements based on offsets
                size_t num = std::min(num_l, num_r);

                for (int i = 0; i < num; ++i)
                {
                    std::iter_swap(first + offsets_l[i], last - offsets_r[i]);
                }

            }

            if (num_l)
            {

            }

            if (num_r)
            {

            }

        }

        std::iter_swap(pivot, i - 1);
        return std::make_pair(i - 1, no_swaps);
    }

    template <typename I, typename Comp>
    static constexpr void pdq_sort_recursive(I first, I last, Comp comp, int depth, bool leftmost)
    {
        const auto size = std::distance(first, last);

        if (size == 0)
        {
            return;
        }

        if (size < InsertionSortThreshold)
        {
            insertion_sort(first, last, comp);
            return;
        }

        auto middle = first + (size >> 1);

        if (size > NintherThreshold)
        {
            sort3(first,      middle,     last - 1,   comp);
            sort3(first + 1,  middle - 1, last - 2,   comp);
            sort3(first + 2,  middle + 1, last - 3,   comp);
            sort3(middle - 1, middle,     middle + 1, comp);
            std::iter_swap(first, middle);
        }
        else
        {
            sort3(first, middle, last - 1, comp);
        }

        // Intervals => [left1, ..., first - 1] [first, ... ]
        // If *(first - 1) == *(first), all elements in
        // the right part are not less than than left part.
        if (!leftmost && !comp(*(first - 1), *first))
        {
            // All the elements between [first, pivot] are equal.
            auto pivot = partition_left(first, last, comp);

            // After partitioning, we have:
            // [left1, ..., first - 1] [x1, x2, ..., pivot, ... ]
            // *(first - 1) <= x1 <= x2 <= ... <= pivot
            // Since (first - 1) == pivot ,so all elements in the left 
            // part are equal to the pivot.
            pdq_sort_recursive(pivot + 1, last, comp, depth, false);

            // This branch will accelerate the case when the elements are all equal.
            return;
        }

        auto [pivot, already_partitioned] = partition_right(first, last, comp);

        const auto lsize = std::distance(first, pivot);
        const auto rsize = std::distance(pivot + 1, last);
        bool highly_unbalanced = lsize < size / 8 || rsize < size / 8;
        
        if (highly_unbalanced)
        {
            if (--depth == 0)
            {   
                heap_sort(first, last, comp);
                return;
            }

            if (lsize >= InsertionSortThreshold)
            {
                std::iter_swap(first, first + lsize / 4);
                std::iter_swap(pivot - 1, pivot - lsize / 4);
                
                if (lsize > NintherThreshold)
                {
                    std::iter_swap(first + 1, first + (lsize / 4 + 1));
                    std::iter_swap(first + 2, first + (lsize / 4 + 2));
                    std::iter_swap(pivot - 2, pivot - (lsize / 4 + 1));
                    std::iter_swap(pivot - 3, pivot - (lsize / 4 + 2));
                }
            }

            if (rsize >= InsertionSortThreshold)
            {
                std::iter_swap(pivot + 1, pivot + (1 + rsize / 4));
                std::iter_swap(last - 1, last - rsize / 4);
                
                if (rsize > NintherThreshold)
                {
                    std::iter_swap(pivot + 2, pivot + (2 + rsize / 4));
                    std::iter_swap(pivot + 3, pivot + (3 + rsize / 4));
                    std::iter_swap(last - 2, last - (1 + rsize / 4));
                    std::iter_swap(last - 3, last - (2 + rsize / 4));
                }
            }
        }
        else
        {
            if (already_partitioned)
            {
                bool result = partial_insertion_sort(first, pivot, comp) && partial_insertion_sort(pivot + 1, last, comp);
                
                if (result) 
                {
                    return; // Already sorted.
                }
            }

            // if (already_partitioned && partial_insertion_sort(first, pivot, comp) && partial_insertion_sort(pivot + 1, last, comp)) 
            // {
            //     return; // Already sorted.
            // }
        }

        pdq_sort_recursive(first, pivot, comp, depth, leftmost);
        pdq_sort_recursive(pivot + 1, last, comp, depth, false);
    }

    template<class Iter, class Compare>
    static constexpr bool partial_insertion_sort(Iter begin, Iter end, Compare comp) {
        typedef typename std::iterator_traits<Iter>::value_type T;
        if (begin == end) return true;
        
        std::size_t limit = 0;
        for (Iter cur = begin + 1; cur != end; ++cur) {
            Iter sift = cur;
            Iter sift_1 = cur - 1;

            // Compare first so we can avoid 2 moves for an element already positioned correctly.
            if (comp(*sift, *sift_1)) {
                T tmp = std::move(*sift);

                do { *sift-- = std::move(*sift_1); }
                while (sift != begin && comp(tmp, *--sift_1));

                *sift = std::move(tmp);
                limit += cur - sift;
            }
            
            if (limit > PartialInsertionSortLimit) return false;
        }

        return true;
    }

public:

    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {
        using DifferenceType = typename std::iterator_traits<I>::difference_type;
        const DifferenceType n = std::distance(first, last);
        const auto max_depth = std::countl_zero(std::make_unsigned_t<DifferenceType>(n)) << 1; // 2 * log2(n)
        return pdq_sort_recursive(first, last, comp, max_depth, true);
    }

};

}

namespace cpp::ranges
{

inline constexpr detail::sorter<detail::pdq_sorter<>> pdq_sort;

}

