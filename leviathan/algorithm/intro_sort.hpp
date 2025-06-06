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

template <int InsertionSortThreshold = 16, pivot_mode Mode = pivot_mode::median3, bool Unguarded = false> 
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

    template<class T>
     static constexpr T* align_cacheline(T* p) {
#if defined(UINTPTR_MAX) && __cplusplus >= 201103L
        std::uintptr_t ip = reinterpret_cast<std::uintptr_t>(p);
#else
        std::size_t ip = reinterpret_cast<std::size_t>(p);
#endif
        ip = (ip + cacheline_size - 1) & -cacheline_size;
        return reinterpret_cast<T*>(ip);
    }

    template<class Iter>
    static constexpr void swap_offsets(Iter first, Iter last,
                             unsigned char* offsets_l, unsigned char* offsets_r,
                             size_t num, bool use_swaps) {
        typedef typename std::iterator_traits<Iter>::value_type T;
        if (use_swaps) {
            // This case is needed for the descending distribution, where we need
            // to have proper swapping for pdqsort to remain O(n).
            for (size_t i = 0; i < num; ++i) {
                std::iter_swap(first + offsets_l[i], last - offsets_r[i]);
            }
        } else if (num > 0) {
            Iter l = first + offsets_l[0]; Iter r = last - offsets_r[0];
            T tmp(std::move(*l)); *l = std::move(*r);
            for (size_t i = 1; i < num; ++i) {
                l = first + offsets_l[i]; *r = std::move(*l);
                r = last - offsets_r[i]; *l = std::move(*r);
            }
            *r = std::move(tmp);
        }
    }

    enum {
        block_size = 64,

        // Cacheline size, assumes power of two.
        cacheline_size = 64
        
    };

    template<class Iter, class Compare>
    static constexpr std::pair<Iter, bool> partition_right_branchless(Iter begin, Iter end, Compare comp) {
        typedef typename std::iterator_traits<Iter>::value_type T;

        sort3(begin, begin + std::distance(begin, end) / 2, end - 1, comp);

        // Move pivot into local for speed.
        T pivot(std::move(*begin));
        Iter first = begin;
        Iter last = end;

        // Find the first element greater than or equal than the pivot (the median of 3 guarantees
        // this exists).
        while (comp(*++first, pivot));

        // Find the first element strictly smaller than the pivot. We have to guard this search if
        // there was no element before *first.
        if (first - 1 == begin) while (first < last && !comp(*--last, pivot));
        else                    while (                !comp(*--last, pivot));

        // If the first pair of elements that should be swapped to partition are the same element,
        // the passed in sequence already was correctly partitioned.
        bool already_partitioned = first >= last;
        if (!already_partitioned) {
            std::iter_swap(first, last);
            ++first;

            // The following branchless partitioning is derived from "BlockQuicksort: How Branch
            // Mispredictions don’t affect Quicksort" by Stefan Edelkamp and Armin Weiss, but
            // heavily micro-optimized.
            unsigned char offsets_l_storage[block_size + cacheline_size];
            unsigned char offsets_r_storage[block_size + cacheline_size];
            unsigned char* offsets_l = align_cacheline(offsets_l_storage);
            unsigned char* offsets_r = align_cacheline(offsets_r_storage);

            Iter offsets_l_base = first;
            Iter offsets_r_base = last;
            size_t num_l, num_r, start_l, start_r;
            num_l = num_r = start_l = start_r = 0;
            
            while (first < last) {
                // Fill up offset blocks with elements that are on the wrong side.
                // First we determine how much elements are considered for each offset block.
                size_t num_unknown = last - first;
                size_t left_split = num_l == 0 ? (num_r == 0 ? num_unknown / 2 : num_unknown) : 0;
                size_t right_split = num_r == 0 ? (num_unknown - left_split) : 0;

                // Fill the offset blocks.
                if (left_split >= block_size) {
                    for (size_t i = 0; i < block_size;) {
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                    }
                } else {
                    for (size_t i = 0; i < left_split;) {
                        offsets_l[num_l] = i++; num_l += !comp(*first, pivot); ++first;
                    }
                }

                if (right_split >= block_size) {
                    for (size_t i = 0; i < block_size;) {
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                    }
                } else {
                    for (size_t i = 0; i < right_split;) {
                        offsets_r[num_r] = ++i; num_r += comp(*--last, pivot);
                    }
                }

                // Swap elements and update block sizes and first/last boundaries.
                size_t num = std::min(num_l, num_r);
                swap_offsets(offsets_l_base, offsets_r_base,
                             offsets_l + start_l, offsets_r + start_r,
                             num, num_l == num_r);
                num_l -= num; num_r -= num;
                start_l += num; start_r += num;

                if (num_l == 0) {
                    start_l = 0;
                    offsets_l_base = first;
                }
                
                if (num_r == 0) {
                    start_r = 0;
                    offsets_r_base = last;
                }
            }

            // We have now fully identified [first, last)'s proper position. Swap the last elements.
            if (num_l) {
                offsets_l += start_l;
                while (num_l--) std::iter_swap(offsets_l_base + offsets_l[num_l], --last);
                first = last;
            }
            if (num_r) {
                offsets_r += start_r;
                while (num_r--) std::iter_swap(offsets_r_base - offsets_r[num_r], first), ++first;
                last = first;
            }
        }

        // Put the pivot in the right place.
        Iter pivot_pos = first - 1;
        *begin = std::move(*pivot_pos);
        *pivot_pos = std::move(pivot);

        return std::make_pair(pivot_pos, already_partitioned);
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
        
        // auto pivot = partition_pivot(first, last, comp);
        auto pivot = partition_right_branchless(first, last, comp).first;
        intro_sort_recursive(first, pivot, comp, depth - 1);
        intro_sort_recursive(pivot + 1, last, comp, depth - 1);
    }

    template <typename I, typename Comp>
    static constexpr void intro_sort_recursive_with_leftmost(I first, I last, Comp comp, int depth, bool leftmost)
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
        
        const auto pivot = partition_pivot(first, last, comp);
        intro_sort_recursive_with_leftmost(first, pivot, comp, depth - 1, leftmost);
        intro_sort_recursive_with_leftmost(pivot + 1, last, comp, depth - 1, false);
    }

public:

    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {
        using DifferenceType = typename std::iterator_traits<I>::difference_type;
        const DifferenceType n = std::distance(first, last);
        const auto max_depth = std::countl_zero(std::make_unsigned_t<DifferenceType>(n)) << 1; // 2 * log2(n)

        if constexpr (Unguarded)
        {
            intro_sort_recursive_with_leftmost(first, last, comp, max_depth, true);
        }
        else
        {
            intro_sort_recursive(first, last, comp, max_depth);
        }
    }
};

}  // namespace cpp::ranges::detail

namespace cpp::ranges
{

inline constexpr detail::sorter<detail::intro_sorter<>> intro_sort; 
inline constexpr detail::sorter<detail::intro_sorter<16, detail::pivot_mode::median3, true>> intro_sort_unguarded; 

}
