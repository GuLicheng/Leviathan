#pragma once

#include "common.hpp"

namespace leviathan::algorithm
{
    template <typename I, typename Comp = std::less<>>
    constexpr void insertion_sort(I first, I last, Comp comp = {}) 
    {
        if (first == last)
        {
            return;
        }

        auto i = first + 1;
        
        for (; i != last; ++i)
        {
            // if arr[j - 1] <= arr[i] continue
            if (!comp(*i, *(i - 1))) 
            {
                continue;
            }
            else
            {
                auto pos = std::upper_bound(first, i, *i, comp);
                auto tmp = std::move(*i);
                std::move_backward(pos, i, i + 1);
                *pos = std::move(tmp);
            } 
        }
    }

    template <typename I, typename Comp = std::less<>>
    constexpr void merge_sort(I first, I last, Comp comp = {})
    {
        if (last - first > 1)
        {
            auto middle = first + (last - first) / 2;
            merge_sort(first, middle, comp);
            merge_sort(middle, last, comp);
            std::inplace_merge(first, middle, last, comp);
        }
    }

    template <typename I, typename Comp = std::less<>>
    constexpr void heap_sort(I first, I last, Comp comp = {})
    {
        std::make_heap(first, last, comp);
        std::sort_heap(first, last, comp);
    }

    /*
        Sorts [begin, end) using insertion sort with the given comparison function. Assumes
        *(begin - 1) is an element smaller than or equal to any element in [begin, end).
    */
    template <typename I, typename Comp = std::less<>>
    constexpr void unguarded_insertion_sort(I first, I last, Comp comp = {})
    {
        if (first == last)
            return;

        for (auto i = first + 1; i != last; ++i)
        {
            auto j = i - 1;
            // 0 [2, 1]
            // ^  ^
            // j  i
            if (comp(*i, *j))
            {
                auto tmp = std::move(*i);
                do { *i-- = std::move(*j--); }
                while (comp(tmp, *j)); // this loop will stop since *(begin-1) is less than any element in [begin, end)
                *i = std::move(tmp);
            }
        }
    } 
} // namespace leviathan

