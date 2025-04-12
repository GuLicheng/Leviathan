#if 0
#pragma once

#include "common.hpp"

#include <algorithm>    // some base algorithms
#include <vector>       // buffer for some algorithms
#include <functional>   // std::invoke, std::less<>
#include <array>        // table
#include <new>
#include <bit>
#include <type_traits>

namespace cpp::sort
{
    // for IntroSort
    namespace detail
    {

        template <typename I, typename Comp>
        constexpr void sort_three(I first, I middle, I last, Comp comp)
        {
            if (comp(*middle, *first)) std::iter_swap(middle, first);
            if (comp(*last, *middle)) 
            {
                std::iter_swap(last, middle);
                if (comp(*middle, *first))
                    std::iter_swap(middle, first);
            }
        }

        // [first, last] and last - first + 1 >= InsertionSortThreadhold
        // we put middle element into prev of last and return it as pivot
        // the three elements will be sorted
        template <typename I, typename Comp>
        constexpr I median_three(I first, I last, Comp comp)
        {
            // [first, middle, last]
            // keep first < middle < last
            auto middle = first + ((last - first) >> 1);
            // if (comp(*middle, *first)) std::iter_swap(first, middle);
            // if (comp(*last, *first)) std::iter_swap(first, last);
            // if (comp(*last, *middle)) std::iter_swap(middle, last);

            sort_three(first, middle, last, comp);
            --last;
            std::iter_swap(middle, last);
            return last;
        }

        // [first, last] and last - first + 1 >= InsertionSortThreadhold
        // we put middle element into first position and return it as pivot
        // the three elements will not be sorted
        template <typename I, typename Comp>
        constexpr I median_three_v2(I first, I last, Comp comp)
        {
            auto result = first;
            ++first;
            auto middle = first + ((last - first) >> 1);

            if (comp(first, middle))
            {
                if (comp(middle, last)) 
                    std::iter_swap(result, middle);
                else if (comp(first, last))
                    std::iter_swap(result, last);
                else
                    std::iter_swap(result, first);
            }
            else if (comp(first, last))
                std::iter_swap(result, first);
            else if (comp(middle, last))
                std::iter_swap(result, last);
            else
                std::iter_swap(result, middle);

            return result;
        }


        // [first, last] and last - first + 1 >= 9
        template <typename I, typename Comp>
        constexpr I median_nine(I first, I last, Comp comp)
        {
            // [first, middle, last]
            // keep first < middle < last
            auto middle = first + ((last - first) >> 1);

            sort_three(first + 1, middle - 1, last - 1, comp);
            sort_three(first + 2, middle + 1, last - 2, comp);
            sort_three(first,     middle,     last,     comp);
            sort_three(middle - 1, middle, middle + 1, comp);
            --last;
            std::iter_swap(middle, last);
            return last;
        } 



        template <typename I, typename Comp>
        constexpr void intro_sort_recursive(I first, I last, Comp comp, int depth)
        {
            const auto dist = last - first; 
            if (dist < (int)constant::IntroSortThreshold)
                return sort::insertion_sort(first, last, comp);

            if (depth == 0)
                return sort::heap_sort(first, last, comp);

            const auto pivot = median_three(first, last - 1, comp);
            // const auto pivot = median_three_v2(first, last - 1, comp);

            auto i = first, j = pivot;

            while (1) 
            {
                while (comp(*(++i), *pivot));
                while (comp(*pivot, *(--j)));
                if (i < j) std::iter_swap(i, j);
                else break;
            }
            depth--;
            std::iter_swap(i, pivot);
            intro_sort_recursive(first, i, comp, depth);
            intro_sort_recursive(i + 1, last, comp, depth);
        }

        int count_left_zero(unsigned long long x)
        {
            int r = 0;
            if (!(x & 0xFFFFFFFF00000000)) r += 32, x <<= 32;
            if (!(x & 0xFFFF000000000000)) r += 16, x <<= 16;
            if (!(x & 0xFF00000000000000)) r += 8,  x <<= 8;
            if (!(x & 0xF000000000000000)) r += 4,  x <<= 4;
            if (!(x & 0xC000000000000000)) r += 2,  x <<= 2;
            if (!(x & 0x8000000000000000)) r += 1,  x <<= 1;
            return r;
        }

        template <typename I, typename Comp = std::less<>>
        constexpr void intro_sort_iteration(I first, I last, Comp comp = {})
        {
            if (last - first < (int)constant::IntroSortThreshold)
                return insertion_sort(first, last, comp);
            
            std::vector stack { first, last };
            // for ascending sequence, f(x) = 2x + 2
            // for descending sequence, f(x) = 2x
            // for random seq, f(x) = 1.5x approximately
            const auto max_depth = count_left_zero(static_cast<std::size_t>(last - first)) * 4;
            stack.reserve(max_depth);
            I split;

            while (stack.size())
            {
                auto plast = stack.back(); stack.pop_back();
                auto pfirst = stack.back(); stack.pop_back();
                const auto dist = plast - pfirst;
                split = plast;

                // if (dist < (int)constant::IntroSortThreshold) 
                // {
                //     // insertion_sort(pfirst, plast);
                //     continue;
                // }
                if (stack.size() == max_depth)
                {
                    heap_sort(pfirst, plast, comp);
                    continue;
                }

                const auto pivot = median_three(pfirst, plast - 1, comp);
                // [first, ..., pivot(middle), last-1, last) first < middle < last - 1
                auto i = pfirst, j = pivot;

                while (1) 
                {
                    while (comp(*(++i), *pivot));
                    while (comp(*pivot, *(--j)));
                    if (i < j) std::iter_swap(i, j);
                    else break;
                }
                std::iter_swap(i, pivot);

                // [pfirst, i)
                if (i - pfirst >= (int)constant::IntroSortThreshold) 
                {
                    stack.push_back(pfirst);
                    stack.push_back(i);
                }

                // [i + 1, plast)

                if (plast - (i + 1) >= (int)constant::IntroSortThreshold) 
                {
                    stack.push_back(i + 1);
                    stack.push_back(plast);
                }
            }
        
            insertion_sort(first, split, comp);
            detail::unguarded_insertion_sort(split, last, comp);
        }

    }

    template <typename I, typename Comp = std::less<>>
    constexpr void intro_sort_iteration(I first, I last, Comp comp = {})
    {
        detail::intro_sort_iteration(first, last, comp);
    }

    template <typename I, typename Comp = std::less<>>
    constexpr void intro_sort_recursive(I first, I last, Comp comp = {})
    {
        const auto max_depth = detail::count_left_zero(static_cast<std::size_t>(last - first)) * 2;
        detail::intro_sort_recursive(first, last, comp, max_depth);
    }

}

#endif