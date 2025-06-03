// https://www.freesion.com/article/2246255399/
// https://github.com/openjdk/jdk/blob/master/src/java.base/share/classes/java/util/TimSort.java

#pragma once

#include "basic_sort.hpp"

namespace cpp::ranges::detail
{

template <int TimSortThreshold = 32> 
class tim_sorter
{   
    template <typename I, typename Comp>
    static constexpr I count_run_and_make_ascending(I first, I last, Comp comp) 
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
    static constexpr void merge_collapse(std::vector<Iterator>& runs, Comp comp)
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
    static constexpr void insertion_sort_rest(I first, I middle, I last, Comp comp)
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
    static constexpr void merge_force_collapse(std::vector<T>& runs, Comp comp)
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

public:

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
            auto next_pos = count_run_and_make_ascending(iter, last, comp);
    
            if (next_pos - iter < min_run)
            {
                const auto dist = last - iter;
                const auto force = std::min(dist, min_run);
                auto tail = iter + force;
                insertion_sort_rest(iter, next_pos, tail, comp);
                next_pos = tail;
            }
    
            stack.emplace_back(next_pos);
            merge_collapse(stack, comp);
            iter = next_pos;
    
        } while (iter != last);
    
        merge_force_collapse(stack, comp);
    }
};

} // namespace cpp::ranges::detail

namespace cpp::ranges
{

inline constexpr detail::sorter<detail::tim_sorter<>> tim_sort;

}
