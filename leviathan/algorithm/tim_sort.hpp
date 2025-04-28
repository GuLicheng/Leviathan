// https://www.freesion.com/article/2246255399/
// http://cr.openjdk.java.net/~martin/webrevs/jdk7/timsort/raw_files/new/src/share/classes/java/util/TimSort.java

#pragma once

#include "basic_sort.hpp"

namespace cpp::ranges::detail
{

template <int TimSortThreshold = 32> 
struct tim_sort_helper
{
    template <typename I, typename Comp>
    static constexpr I count_run_and_make_ascending(I first, I last, Comp& comp) 
    {
        // comp is less relationship
        if (first == last)
        {
            return first; // something may not happened
        }
    
        auto left = first;
        auto right = left + 1;
    
        if (right == last)
        {
            return right;
        }
    
        if (comp(*right, *left))
        {
            // (first > last  <=>  last < first ) ++ and reverse
            do { ++right; } 
            while (right != last && comp(*right, *(right - 1)));
            std::reverse(first, right);
        }
        else
        {
            // while first <= last ++
            do { ++right; } 
            while (right != last && !comp(*right, *(right - 1)));
        }
        return right;
    }
        
    template <typename T>
    static constexpr T min_run_length(T n) 
    {
        T r = 0;  
    
        while (n >= TimSortThreshold) 
        {
            r |= (n & 1);
            n >>= 1;
        }
        
        return n + r;
    }
    
    template <typename T, typename Comp>
    static constexpr void merge_collapse(std::vector<T>& runs, Comp& comp)
    {
        while (runs.size() > 2)
        {
            if (runs.size() == 3)
            {
                // only have two runs
                auto left = runs[0];
                auto middle = runs[1];
                auto right = runs[2];
                
                if (middle - left <= right - middle)
                {
                    std::inplace_merge(left, middle, right, comp);
                    runs.erase(runs.begin() + 1); // remove middle
                }
                else 
                {
                    break;
                }
            }
            else
            {
                auto last_pos = runs.end();
                auto iter4 = *(last_pos - 1);
                auto iter3 = *(last_pos - 2);
                auto iter2 = *(last_pos - 3);
                auto iter1 = *(last_pos - 4);
                const auto Z = std::distance(iter1, iter2);
                const auto Y = std::distance(iter2, iter3);
                const auto X = std::distance(iter3, iter4); // stack top
    
                if (X + Y < Z && X < Y)
                {
                    break;
                }
                else
                {
                    if (Z < X) // merge first 2
                    {
                        std::inplace_merge(iter1, iter2, iter3, comp);
                        runs.erase(last_pos - 3);
                    }
                    else // merge last 2
                    {
                        std::inplace_merge(iter2, iter3, iter4, comp);
                        runs.erase(last_pos - 2);
                    }
                }
            }
        }
    }
    
    template <typename T, typename Comp>
    static constexpr void merge_force_collapse(std::vector<T>& runs, Comp& comp)
    {
        const int sz = static_cast<int>(runs.size());
    
        for (int i = sz - 2; i >= 1; --i)
        {
            std::inplace_merge(runs[i - 1], runs[i], runs[sz - 1], comp);
        }
    }
    
    template <typename I, typename Comp>
    static constexpr void tim_sort_impl(I first, I last, Comp comp)
    {
        if (last - first < TimSortThreshold)
        {
            return insertion_sort(first, last, comp);
        }
    
        auto iter = first; 
        std::vector stack{ iter };
        stack.reserve(64);  // dynamic buffer, avoid overflow
    
        // min length
        const auto remaining = std::distance(iter, last);
        const auto min_run = min_run_length(remaining);
        do 
        {
            auto next_iter = count_run_and_make_ascending(iter, last, comp);
    
            if (next_iter - iter < min_run)
            {
                const auto dist = last - iter;
                auto force = std::min(dist, min_run);
                next_iter = iter + force;
                insertion_sort(iter, next_iter, comp);
            }
    
            stack.emplace_back(next_iter);
            merge_collapse(stack, comp);
            iter = next_iter;
    
        } while (iter != last);
    
        merge_force_collapse(stack, comp);
    }
};

} // namespace cpp::ranges::detail

namespace cpp::ranges
{

inline constexpr struct
{
    template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
    static constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) 
    {
        auto tail = std::ranges::next(first, last);
        detail::tim_sort_helper<32>::tim_sort_impl(first, tail, detail::make_comp_proj(comp, proj));
        return tail;    
    }   

    template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity> 
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj> 
    constexpr std::ranges::borrowed_iterator_t<Range> 
    static operator()(Range &&r, Comp comp = {}, Proj proj = {})  
    {
        return operator()(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }
} tim_sort;

}
