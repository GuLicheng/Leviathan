#pragma once

#include "common.hpp"

#include <algorithm>    // some base algorithms
#include <vector>       // buffer for some algorithms
#include <functional>   // std::invoke, std::less<>
#include <concepts>     // some concepts such as std::sortable and std::random_access_iterator
#include <array>        // table
#include <new>
#include <bit>
#include <type_traits>

namespace leviathan::sort
{

    /*----------------------------Some basic sort algorithms------------------------------------*/

    template <typename I, typename Comp = std::less<>>
    constexpr void insertion_sort(I first, I last, Comp comp = {}) 
    {
        if (first == last)
            return;

        auto i = first + 1;
        for (; i != last; ++i)
        {
            // if arr[j - 1] <= arr[i] continue
            if (!comp(*i, *(i - 1))) continue;
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

    template <typename I, typename Comp = std::less<>>
    constexpr void quick_sort(I first, I last, Comp comp = {})
    {
        if (first == last || first + 1 == last) return;

        auto i = first - 1, j = last;
        const auto offset = (std::distance(first, last) - 1) >> 1;
        const auto x = *(first + offset);
        while (i < j) 
        {
            while (comp(*(++i), x));
            while (comp(x, *(--j)));
            if (i < j) std::swap(*i, *j);
        }
        quick_sort(first, j + 1, comp);
        quick_sort(j + 1, last, comp);
    }


    /*----------------------------hybrid sort algorithms------------------------------------*/

    enum struct constant : int
    {
        TimSortThreshold = 32,
        IntroSortThreshold = 16,
        MedianNineThreshold = 128
    };

    // for TimSort
    namespace detail
    {

        template <typename I, typename Comp>
        constexpr I count_run_and_make_ascending(I first, I last, Comp comp) 
        {
            // comp is less relationship
            if (first == last)
                return first; // something may not happened

            auto left = first;
            auto right = left + 1;

            if (right == last)
                return right;

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
        constexpr T min_run_length(T n) 
        {
            T r = 0;      
            while (n >= (int)constant::TimSortThreshold) 
            {
                r |= (n & 1);
                n >>= 1;
            }
            return n + r;
        }

        template <typename T, typename Comp>
        constexpr void merge_collapse(std::vector<T>& runs, Comp comp)
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
                        break;
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
                        break;
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
        constexpr void merge_force_collapse(std::vector<T>& runs, Comp comp)
        {
            const int sz = static_cast<int>(runs.size());
            for (int i = sz - 2; i >= 1; --i)
                std::inplace_merge(runs[i - 1], runs[i], runs[sz - 1], comp);
        }

    }

    template <typename I, typename Comp = std::less<>>
    constexpr void tim_sort(I first, I last, Comp comp = {})
    {
        if (last - first < (int)constant::TimSortThreshold)
            return insertion_sort(first, last, comp);

        auto iter = first; 
        std::vector stack{ iter };
        stack.reserve(64);  // dynamic buffer, avoid overflow

        // min length
        const auto remaining = std::distance(iter, last);
        const auto min_run = detail::min_run_length(remaining);
        do 
        {
            auto next_iter = detail::count_run_and_make_ascending(iter, last, comp);
            if (next_iter - iter < min_run)
            {
                const auto dist = last - iter;
                auto force = std::min(dist, min_run);
                next_iter = iter + force;
                insertion_sort(iter, next_iter, comp);
            }
            stack.emplace_back(next_iter);
            detail::merge_collapse(stack, comp);
            iter = next_iter;

        } while (iter != last);

        detail::merge_force_collapse(stack, comp);
    }

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

        /*
            Sorts [begin, end) using insertion sort with the given comparison function. Assumes
            *(begin - 1) is an element smaller than or equal to any element in [begin, end).
        */
        template <typename I, typename Comp>
        constexpr void unguarded_insertion_sort(I first, I last, Comp comp)
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

namespace leviathan
{

#define RegisterSortAlgorithm(name) \
    struct name##_fn {  \
        template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity> \
        requires std::sortable<I, Comp, Proj> \
        constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const \
        { sort:: name (std::move(first), std::move(last), detail::make_comp_proj(comp, proj)); return first + (last - first); }            \
        template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity> \
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj> \
        constexpr std::ranges::borrowed_iterator_t<Range> \
        operator()(Range &&r, Comp comp = {}, Proj proj = {}) const \
        { return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj)); } \
    } ; \
    inline constexpr name##_fn name{}

    // binary insertion sort
    RegisterSortAlgorithm(insertion_sort);

    RegisterSortAlgorithm(merge_sort);

    // https://www.freesion.com/article/2246255399/
    // http://cr.openjdk.java.net/~martin/webrevs/jdk7/timsort/raw_files/new/src/share/classes/java/util/TimSort.java
    RegisterSortAlgorithm(tim_sort);

    RegisterSortAlgorithm(heap_sort);

    RegisterSortAlgorithm(quick_sort);

    // https://en.wikipedia.org/wiki/Introsort
    // Musser, D.: Introspective sorting and selection algorithms. Software Practice and Experience 27, 983–993 (1997)
    // http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.14.5196&rep=rep1&type=pdf
    RegisterSortAlgorithm(intro_sort_recursive);
    RegisterSortAlgorithm(intro_sort_iteration);


#undef RegisterSortAlgorithm


#define RegisterSTLSortAlgorithm(name) \
    struct name##_fn {  \
        template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity> \
        requires std::sortable<I, Comp, Proj> \
        constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const \
        { std:: name (std::move(first), std::move(last), detail::make_comp_proj(comp, proj)); return first + (last - first); }            \
        template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity> \
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj> \
        constexpr std::ranges::borrowed_iterator_t<Range> \
        operator()(Range &&r, Comp comp = {}, Proj proj = {}) const \
        { return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj)); } \
    } ; 

    RegisterSTLSortAlgorithm(sort);

#undef RegisterSTLSortAlgorithm


} // namespace leviathan



/*
https://www.acwing.com/problem/content/description/787/

Code Template:

#pragma GCC optimize ("O3")

#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
#include <iterator>

type your sort functions here

int main()
{
    using namespace leviathan::sort;
    std::ios::sync_with_stdio(false);
    std::vector<int> v;
    int n; std::cin >> n;
    v.reserve(n);
    std::copy(std::istream_iterator<int>{std::cin}, std::istream_iterator<int>{}, std::back_inserter(v));

    // your sort algorithm
    heap_sort(v.begin(), v.end());  
    
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>{std::cout, " "});
    std::endl(std::cout);
    return 0;
}
*/