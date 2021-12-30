#ifndef __ALGORITHM_HPP__
#define __ALGORITHM_HPP__

#include <algorithm>    // some base algorithms
#include <vector>       // buffer for some algorithms
#include <functional>   // std::invoke, std::less<>
#include <concepts>     // some concepts such as std::sortable and std::random_access_iterator
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
            auto j = i - 1;
            // if arr[j] <= arr[i] continue
            if (!comp(*i, *j)) continue;
            else
            {
                auto pos = std::upper_bound(first, i, *i, comp);
                auto tmp = std::move(*i);
                std::move(pos, i, pos + 1);
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
        while (first != last)
            std::pop_heap(first, last--, comp);
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
        IntroSortThreshold = 16
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
            return insertion_sort(std::move(first), std::move(last), std::move(comp));

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

    namespace detail
    {

    #ifdef __cpp_lib_hardware_interference_size
        using std::hardware_constructive_interference_size;
        using std::hardware_destructive_interference_size;
    #else
        // 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
        // https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
        constexpr std::size_t hardware_constructive_interference_size = 64;
        constexpr std::size_t hardware_destructive_interference_size = 64;
    #endif

        inline constexpr int CacheLineSize = hardware_constructive_interference_size;
        inline constexpr int InsertionSortThreshold = 24;
        inline constexpr int BlockSize = 64;

        template <typename Pointer>
        constexpr bool is_cache_aligned(Pointer* ptr) 
        {
            return static_cast<std::uintptr_t>(ptr) % CacheLineSize;
        }

        // [first, last] and last - first + 1 >= 3
        template <typename I, typename Comp>
        constexpr I median_three(I first, I last, Comp comp)
        {
            // [first, middle, last]
            // keep first < middle < last
            auto middle = first + ((last - first) >> 1);
            // if (comp(*middle, *first)) std::iter_swap(first, middle);
            // if (comp(*last, *first)) std::iter_swap(first, last);
            // if (comp(*last, *middle)) std::iter_swap(middle, last);

            if (comp(*middle, *first)) std::iter_swap(middle, first);
            if (comp(*last, *middle)) 
            {
                std::iter_swap(last, middle);
                if (comp(*middle, *first))
                    std::iter_swap(middle, first);
            }
            --last;
            std::iter_swap(middle, last);
            return last;
        }

        template <typename I, typename Comp>
        constexpr void intro_sort_loop(I first, I last, Comp comp, int depth)
        {
            const auto dist = last - first; 
            if (dist <= (int)constant::IntroSortThreshold)
                return sort::insertion_sort(first, last, comp);
            if (depth == 0)
                return sort::heap_sort(first, last, comp);

            const auto pivot = median_three(first, --last, comp);
            auto i = first, j = last - 1;

            while (1) 
            {
                while (comp(*(++i), *pivot));
                while (comp(*pivot, *(--j)));
                if (i < j) std::iter_swap(i, j);
                else break;
            }
            depth--;
            std::iter_swap(i, last - 1);
            intro_sort_loop(first, i, comp, depth);
            intro_sort_loop(i + 1, last + 1, comp, depth);
        }

        template <typename I, typename Comp>
        constexpr void intro_sort_iteration(I first, I last, Comp comp, int depth)
        {
            const auto dist = last - first; 
            if (dist <= (int)constant::IntroSortThreshold)
                return;
            if (depth == 0)
                return sort::heap_sort(first, last, comp);

            const auto pivot = median_three(first, --last, comp);
            auto i = first, j = last - 1;

            while (1) 
            {
                while (comp(*(++i), *pivot));
                while (comp(*pivot, *(--j)));
                if (i < j) std::iter_swap(i, j);
                else break;
            }
            depth--;
            std::iter_swap(i, last - 1);
            intro_sort_loop2(first, i, comp, depth);
            intro_sort_loop2(i + 1, last + 1, comp, depth);
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

    }

    template <typename I, typename Comp = std::less<>>
    constexpr void intro_sort(I first, I last, Comp comp = {})
    {
        #ifdef __cpp_lib_bitops
        const auto max_depth = std::countl_zero(static_cast<std::size_t>(last - first));
        #else
        const auto max_depth = detail::count_left_zero(static_cast<unsigned long long>(last - first));
        #endif
        detail::intro_sort_loop(first, last, comp, max_depth * 2);
    }

}


namespace leviathan
{

    // Copy from stl_algo.h
    template <typename Comp, typename Proj>
    constexpr auto make_comp_proj(Comp& comp, Proj& proj)
    {
        return [&](auto&& lhs, auto&& rhs) -> bool
        {
            using _TL = decltype(lhs);
            using _TR = decltype(rhs);
            return std::invoke(comp,
                                 std::invoke(proj, std::forward<_TL>(lhs)),
                                 std::invoke(proj, std::forward<_TR>(rhs)));
        };
    }


#define RegisterSortAlgorithm(name) \
    struct name##_fn {  \
        template <std::random_access_iterator I, std::sentinel_for<I> S, typename Comp = std::ranges::less, typename Proj = std::identity> \
        requires std::sortable<I, Comp, Proj> \
        constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const \
        { sort:: name (std::move(first), std::move(last), make_comp_proj(comp, proj)); return first + (last - first); }            \
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

    RegisterSortAlgorithm(intro_sort);

#undef RegisterSortAlgorithm

    // std::sort -> introsort
    // Musser, D.: Introspective sorting and selection algorithms. Software Practice and Experience 27, 983–993 (1997)
    // http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.14.5196&rep=rep1&type=pdf

} // namespace leviathan

#endif


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