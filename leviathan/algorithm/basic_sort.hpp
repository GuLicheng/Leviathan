#pragma once

#include "common.hpp"
#include <ranges>

namespace cpp::ranges::detail
{
    
template <typename Sorter>
struct sorter
{
    template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto tail = std::ranges::next(first, last);
        Sorter()(first, tail, detail::make_comp_proj(comp, proj));
        return tail;    
    }   

    template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity> 
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj> 
    constexpr std::ranges::borrowed_iterator_t<Range> static operator()(Range&& r, Comp comp = {}, Proj proj = {})  
    {
        return operator()(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }
};

template <typename I, typename Comp>
constexpr void unguarded_linear_insert(I last, Comp comp)
{
    auto val = std::move(*last);
    auto i = last;
    --i;

    while (comp(val, *i))
    {
        *last = std::move(*i);
        last = i;
        --i;
    }

    *last = std::move(val);
}

template <typename I, typename Comp>
constexpr void insertion_sort(I first, I last, Comp comp)
{
    if (first == last) 
    {
        return;
    }

    for (I i = first + 1; i != last; ++i) 
    {
        if (comp(*i, *first)) 
        {
            auto tmp = std::move(*i);
            std::move_backward(first, i, i + 1);
            *first = std::move(tmp);
        }
        else
        {
            unguarded_linear_insert(i, comp);
        }
    }
}

/*
    Sorts [begin, end) using insertion sort with the given comparison function. Assumes
    *(begin - 1) is an element smaller than or equal to any element in [begin, end).
*/
template <typename I, typename Comp>
constexpr void unguarded_insertion_sort(I first, I last, Comp comp)
{
    if (first == last)
    {
        return;
    }

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename I, typename Comp>
constexpr void merge_sort(I first, I last, Comp comp)
{
    if (last - first > 1)
    {
        auto middle = first + (last - first) / 2;
        merge_sort(first, middle, comp);
        merge_sort(middle, last, comp);
        std::inplace_merge(first, middle, last, comp);
    }
}

template <typename I, typename Comp>
constexpr void heap_sort(I first, I last, Comp comp)
{
    std::make_heap(first, last, comp);
    std::sort_heap(first, last, comp);
}


} // namespace cpp

