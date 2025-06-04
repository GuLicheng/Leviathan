// https://www.freesion.com/article/2246255399/
// https://github.com/openjdk/jdk/blob/master/src/java.base/share/classes/java/util/TimSort.java

#pragma once

#include "basic_sort.hpp"

namespace cpp::ranges::detail
{

template <int TimSortThreshold = 32> 
class tim_sorter
{   
protected:

    template <typename I, typename Comp>
    static constexpr I count_run_and_make_ascending(I first, I last, Comp comp) 
    {
        // assert(first != last && "something may not happened");
    
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
    
    template <typename I, typename Comp>
    static constexpr void merge_at(std::vector<I>& runs, typename std::vector<I>::size_type n, Comp comp)
    {
        std::inplace_merge(runs[n], runs[n + 1], runs[n + 2], comp);
        runs.erase(runs.begin() + n + 1);
    }

    template <typename I, typename Comp>
    static constexpr void merge_collapse(std::vector<I>& runs, Comp comp)
    {
        while (runs.size() > 2)
        {
            size_t n = runs.size() - 3; // Y
            auto X = std::distance(runs[n + 1], runs[n + 2]);
            auto Y = std::distance(runs[n], runs[n + 1]);

            if (n > 0 && std::distance(runs[n - 1], runs[n]) <= X + Y)
            {
                // Z <= Y + X
                if (std::distance(runs[n - 1], runs[n]) < X) 
                { 
                    // Z < X
                    --n;
                }
                merge_at(runs, n, comp);
            }
            else if (Y <= X)
            {
                // merge Y and X
                merge_at(runs, n, comp);
            }
            else
            {
                break;
            }
        }
    }
 
    template <typename I, typename Comp>
    static constexpr void insertion_sort_rest(I first, I middle, I last, Comp comp)
    {
        [[assume(first != middle)]];

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
    static constexpr void merge_force_collapse(std::vector<T>& runs, Comp comp)
    {
        if (runs.size() <= 2)
        {
            return; // nothing to collapse
        }

        while (runs.size() > 2)
        {
            size_t n = runs.size() - 3; // Top

            if (n > 0 && std::distance(runs[n - 1], runs[n]) < std::distance(runs[n + 1], runs[n + 2]))
            {
                // [..., Z, Y, X
                n--;
            }

            merge_at(runs, n, comp);
        }
    }

public:

    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {
        if (last - first < TimSortThreshold)
        {
            insertion_sort(first, last, comp);
            return;
        }
    
        auto left = first; 
        std::vector stack{ left };
        stack.reserve(64);  // dynamic buffer, avoid overflow
    
        // min length
        const auto remaining = std::distance(left, last);
        const auto min_run = min_run_length(remaining);

        do 
        {
            auto right = count_run_and_make_ascending(left, last, comp);
    
            if (right - left < min_run)
            {
                auto tail = std::ranges::next(left, min_run, last);
                insertion_sort_rest(left, right, tail, comp);
                right = tail;
            }
            
            stack.emplace_back(right);
            merge_collapse(stack, comp);
            left = right;
    
        } while (left != last);
    
        merge_force_collapse(stack, comp);
    }
};

} // namespace cpp::ranges::detail

namespace cpp::ranges
{

inline constexpr detail::sorter<detail::tim_sorter<>> tim_sort;

}
