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
    bool Branchless = true,
    int InsertionSortThreshold = 24, 
    int NintherThreshold = 128,
    int PartialInsertionSortLimit = 8,
    int BlockSize = 64,
    int CachelineSize = 64> 
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
    static constexpr void median(I first, I last, Comp comp)
    {
        const auto size = std::distance(first, last);
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
    }

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
        auto target = swap_block(i, j, pivot, comp, no_swaps);
        std::iter_swap(target, pivot);
        return std::make_pair(target, no_swaps);
    }

    template <typename I, typename Comp>
    static constexpr I swap_block(I first, I last, I pivot, Comp comp, bool no_swaps)
    {
        if (no_swaps)
        {
            return first - 1;
        }

        I i = first, j = last;

        std::iter_swap(i, j);
        ++i;

        constexpr auto offsets_size = BlockSize + CachelineSize;

        alignas(CachelineSize) unsigned char offsets_l_storage[offsets_size];
        alignas(CachelineSize) unsigned char offsets_r_storage[offsets_size];

        unsigned char* offsets_l = offsets_l_storage;
        unsigned char* offsets_r = offsets_r_storage;

        I offsets_l_base = i;
        I offsets_r_base = j;

        int num_l = 0, num_r = 0;
        int start_l = 0, start_r = 0;

        while (i < j)
        {
            // [1, 2, 3, ..., 3, 4, 2]
            const auto num_known = std::distance(i, j);

            const int left_split = num_l == 0 ? (num_r == 0 ? num_known >> 1 : num_known) : 0;
            const int right_split = num_r == 0 ? (num_known - left_split) : 0;
            
            static_assert(BlockSize % 8 == 0, "BlockSize must be a multiple of 8 for branchless partitioning.");

            if (left_split >= BlockSize)
            {
                for (int k = 0; k < BlockSize;)
                {
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                }
            }
            else
            {
                for (int k = 0; k < left_split;)
                {
                    offsets_l[num_l] = k++; num_l += !comp(*i++, *pivot);
                }
            }

            if (right_split >= BlockSize)
            {
                for (int k = 0; k < BlockSize;)
                {
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                }
            }
            else
            {
                for (int k = 0; k < right_split;)
                {
                    offsets_r[num_r] = ++k; num_r += comp(*--j, *pivot);
                }
            }

            const int num = std::min(num_l, num_r);

            // If the sides are not equal, we swap only the minimum number of elements.
            for (int k = 0; k < num; ++k)
            {
                std::iter_swap(
                    offsets_l_base + offsets_l[start_l + k], 
                    offsets_r_base - offsets_r[start_r + k]
                );
            }

            num_l -= num; num_r -= num;
            start_l += num; start_r += num;

            if (num_l == 0)
            {
                start_l = 0;
                offsets_l_base = i;
            }

            if (num_r == 0)
            {
                start_r = 0;
                offsets_r_base = j;
            }
        }

        if (num_l)
        {
            offsets_l += start_l;
            while (num_l--) std::iter_swap(offsets_l_base + offsets_l[num_l], --j);
            i = j;
        }

        if (num_r)
        {
            offsets_r += start_r;
            while (num_r--) std::iter_swap(offsets_r_base - offsets_r[num_r], i++);
            j = i;
        }

        return i - 1;
    }

    template <typename I, typename Comp>
    static constexpr void pdq_sort_recursive(I first, I last, Comp comp, int depth, bool leftmost)
    {
        const auto size = std::distance(first, last);

        if (size < InsertionSortThreshold)
        {
            insertion_sort(first, last, comp);
            return;
        }

        median(first, last, comp);

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
            if (already_partitioned && partial_insertion_sort(first, pivot, comp) && partial_insertion_sort(pivot + 1, last, comp)) 
            {
                return; // Already sorted.
            }
        }

        pdq_sort_recursive(first, pivot, comp, depth, leftmost);
        pdq_sort_recursive(pivot + 1, last, comp, depth, false);
    }

    template <typename I, typename Comp>
    static constexpr bool partial_insertion_sort(I first, I last, Comp comp)
    {
        if (first == last) 
        {
            return true;
        }

        int limit = 0;

        for (I i = first + 1; i != last; ++i)
        {
            I j = i - 1;

            // Compare first so we can avoid 2 moves for an element already positioned correctly.
            if (comp(*i, *j)) 
            {
                auto val = std::move(*i);
                
                do { *i-- = std::move(*j); }
                while (i != first && comp(val, *--j));

                *i = std::move(val);
                limit += std::distance(i, j);
            }

            if (limit > PartialInsertionSortLimit) 
            {
                // Too many elements moved, abort sorting.
                return false; 
            }
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
        pdq_sort_recursive(first, last, comp, max_depth, true);
    }

};

class pdq_sorter2
{
public:

    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {
        using ValueType = std::iter_value_t<I>;
        constexpr bool Branchless = std::is_arithmetic_v<ValueType>;
        pdq_sorter<Branchless>()(std::move(first), std::move(last), std::move(comp));
    }
};

}

namespace cpp::ranges
{

inline constexpr detail::sorter<detail::pdq_sorter<false>> pdq_sort_branch;
inline constexpr detail::sorter<detail::pdq_sorter<true>> pdq_sort_branchless;
inline constexpr detail::sorter<detail::pdq_sorter2> pdq_sort;

}

