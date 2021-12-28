#ifndef __ALGORITHM_HPP__
#define __ALGORITHM_HPP__

#include <algorithm>    // some base algorithms
#include <vector>       // buffer for some algorithms
#include <functional>   // std::invoke, std::less<>
#include <concepts>     // some concepts such as std::sortable and std::random_access_iterator
#include <new>

namespace leviathan::sort
{

    template <typename I, typename S,  typename Comp = std::less<>>
    constexpr I insertion_sort(I first, S last, Comp comp = {}) 
    {
        if (first == last)
            return first;

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
        return i;
    }

    template <typename I, typename S,  typename Comp = std::less<>>
    constexpr I merge_sort(I first, S last, Comp comp = {})
    {
        if (last - first > 1)
        {
            auto middle = first + (last - first) / 2;
            merge_sort(first, middle, comp);
            merge_sort(middle, last, comp);
            std::inplace_merge(first, middle, last, comp);
        }
        return first;
    }

    template <typename I, typename S, typename Comp = std::less<>>
    constexpr I heap_sort(I first, S last, Comp comp = {})
    {
        std::make_heap(first, last, comp);
        while (first != last)
            std::pop_heap(first, last--, comp);
        return first;
    }

    template <typename I, typename S, typename Comp = std::less<>>
    constexpr I quick_sort(I first, S last, Comp comp = {})
    {
        if (first == last || first + 1 == last) return first;

        auto i = first - 1, j = last;
        const auto offset = (std::distance(first, last) - 1) >> 1;
        auto x = *(first + offset);
        while (i < j) 
        {
            while (comp(*(++i), x));
            while (comp(x, *(--j)));
            if (i < j) std::swap(*i, *j);
        }
        quick_sort(first, j + 1, comp);
        quick_sort(j + 1, last, comp);
        return last;
    }

    // for TimSort
    namespace detail
    {
        inline constexpr int MinSize = 32; 

        template <typename I, typename S, typename Comp>
        constexpr I count_run_and_make_ascending(I first, S last, Comp comp) 
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
            while (n >= detail::MinSize) 
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

    template <typename I, typename S, typename Comp = std::less<>>
    constexpr I tim_sort(I first, S last, Comp comp = {})
    {
        if (last - first < detail::MinSize)
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
                next_iter = insertion_sort(iter, iter + force, comp);
            }
            stack.emplace_back(next_iter);
            detail::merge_collapse(stack, comp);
            iter = next_iter;

        } while (iter != last);

        detail::merge_force_collapse(stack, comp);

        return iter;
    }

    // for pqdsort
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

        template <typename I, typename Comp>
        constexpr void median_three(I first, I middle, I last, Comp comp)
        {
            // [first, middle, last]
            // keep first < middle < last
            if (comp(*middle, *first)) std::iter_swap(first, middle);
            if (comp(*last, *first)) std::iter_swap(first, last);
            if (comp(*last, *middle)) std::iter_swap(middle, last);
        }
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
        { return sort:: name (std::move(first), std::move(last), make_comp_proj(comp, proj)); }            \
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

    // http://cr.openjdk.java.net/~martin/webrevs/jdk7/timsort/raw_files/new/src/share/classes/java/util/TimSort.java
    RegisterSortAlgorithm(tim_sort);

    RegisterSortAlgorithm(heap_sort);

    RegisterSortAlgorithm(quick_sort);

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