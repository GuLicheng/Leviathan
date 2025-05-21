// https://www.freesion.com/article/2246255399/
// http://cr.openjdk.java.net/~martin/webrevs/jdk7/timsort/raw_files/new/src/share/classes/java/util/TimSort.java

#pragma once

#include "basic_sort.hpp"

namespace cpp::ranges::detail
{

template <int TimSortThreshold = 32> 
struct tim_sorter
{   
    template <typename I, typename Comp>
    static constexpr I count_run_and_make_ascending(I first, I last, Comp& comp) 
    {
        assert(first != last && "something may not happened");
    
		auto iter = first + 1;

        if (iter == last)
        {
            return last;
        }

        if (comp(*iter, *first))
        {
            // 2, 1
			do { ++iter; } while (iter != last && comp(*iter, *(iter - 1)));
            std::reverse(first, iter);
        }
        else
        {
			// 1, 2
			do { ++iter; } while (iter != last && !comp(*iter, *(iter - 1)));
        }

		return iter;

        // auto left = first;
        // auto right = left + 1;
    
        // if (right == last)
        // {
        //     return right;
        // }
    
        // if (comp(*right, *left))
        // {
        //     // (first > last  <=>  last < first ) ++ and reverse
        //     do { ++right; } 
        //     while (right != last && comp(*right, *(right - 1)));
        //     std::reverse(first, right);
        // }
        // else
        // {
        //     // while first <= last ++
        //     do { ++right; } 
        //     while (right != last && !comp(*right, *(right - 1)));
        // }
        // return right;
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
    
    template <typename Iterator, typename Comp>
    static constexpr void merge_collapse(std::vector<Iterator>& runs, Comp& comp)
    {
        while (runs.size() > 2)
        {
            size_t n = runs.size() - 3; // Y
            auto last = runs.end();
            auto X = std::distance(*(last - 2), *(last - 1));
            auto Y = std::distance(*(last - 3), *(last - 2)); 

            if (n > 0 && std::distance(*(last - 4), *(last - 3)) <= X + Y)
            {
                // Z <= Y + X
                if (std::distance(*(last - 4), *(last - 3)) < X) 
                {
                    // Z < X
                    --n;
                }
                std::inplace_merge(runs[n], runs[n + 1], runs[n + 2], comp);
                runs.erase(runs.begin() + n + 1); // remove middle
            }
            else if (Y <= X)
            {
                // merge Y and X
                std::inplace_merge(runs[n], runs[n + 1], runs[n + 2], comp);
                runs.erase(runs.begin() + n + 1); // remove middle
            }
            else
            {
                break;
            }
        }
    }
 
    template <typename I, typename Comp>
    static constexpr void insertion_sort_rest(I first, I middle, I last, Comp& comp)
    {
        assert(first != middle);

        for (I i = middle; i != last; ++i)
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

    template <typename T, typename Comp>
    static constexpr void merge_force_collapse(std::vector<T>& runs, Comp& comp)
    {
        while (runs.size() > 2)
        {
            size_t n = runs.size() - 3; 

            if (n > 0 && std::distance(runs[n - 1], runs[n]) <= std::distance(runs[n], runs[n + 1]))
            {
                n--;
            }

            std::inplace_merge(runs[n], runs[n + 1], runs[n + 2], comp);
            runs.erase(runs.begin() + n + 1); // remove middle
        }
    }
    
    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {
        if (last - first < TimSortThreshold)
        {
            insertion_sort(first, last, comp);
            return;
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
                const auto force = std::min(dist, min_run);
                auto tail = iter + force;
                insertion_sort_rest(iter, next_iter, tail, comp);
                // binary_sort_rest(iter, next_iter, tail, comp);
                next_iter = tail;
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
        detail::tim_sorter<>()(first, tail, detail::make_comp_proj(comp, proj));
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
