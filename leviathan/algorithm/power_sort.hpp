/*
    https://github.com/python/cpython/blob/main/Objects/listobject.c
    https://arxiv.org/pdf/1805.04154
    https://github.com/sebawild/powersort/blob/main/src/sorts/powersort.h
    https://github.com/sebawild/nearly-optimal-mergesort-code/tree/master

    POWERSORT(A[1..n]):

    X := stack of runs
    P := stack of powers   
    s1 := 1; e1 = EXTENDRUNRIGHT(A[1..n])

    while e1 < n:
        s2 := e1 + 1; e2 = EXTENDRUNRIGHT(A[s2..n])
        p := NodePower(s1, e1, s2, e2, n)
        while P.top() > p:
            P.pop() 
            (s1, e1) := Merge(X.pop(), A[s1..e1])
        X.push(A[s1, e1]); P.push(p)
        s1 := s2; e1 := e2
    end while

    while ¬X.empty()
        (s1, e1) := Merge(X.pop(), A[s1..e1])

    NodePower(s1, e1, s2, e2, n)
    
    n1 := e1 − s1 + 1; n2 := e2 − s2 + 1; l := 0
    a := (s1 + n1/2 − 1)/n; b := (s2 + n2/2 − 1)/n
    while a * 2^l == b * 2^l do l = l + 1 end while
    return l


    => n1 = e1 - s1 + 1
    => a = (s1 + n1 / 2 - 1) / n
         = (s1 + (e1 - s1 + 1) / 2 - 1) / n
         = ((e1 - s1) + 1 / 2 - 1) / n
         = (n1 - 1) / 2n
         = (e1 - s1) / 2n
        
*/

#pragma once

#include <cmath>

#include "tim_sort.hpp"

namespace cpp::ranges::detail
{

enum class power_policy
{
    // https://github.com/sebawild/nearly-optimal-mergesort-code/tree/master
    java,    
    
    // https://github.com/python/cpython/blob/main/Objects/listobject.c
    python,
};

template <int MinRunLen = 24, power_policy Policy = power_policy::java> 
class power_sorter : public tim_sorter<MinRunLen>
{
protected:

    using base = tim_sorter<MinRunLen>;
    using power_type = uint32_t;
    using base::count_run_and_make_ascending;
    using base::insertion_sort_rest;

    // https://github.com/python/cpython/blob/main/Objects/listobject.c
    template <typename I>
        requires (Policy == power_policy::python)
    static constexpr power_type node_power(I first, I middle1, I middle2, I middle3, I last)
    {
        // run1 => [middle1, middle2)
        // run2 => [middle2, middle3)

        using DifferenceType = typename std::iterator_traits<I>::difference_type;
    
        DifferenceType n = std::distance(first, last);
        DifferenceType s1 = std::distance(first, middle1);
        DifferenceType n1 = std::distance(middle1, middle2);
        DifferenceType n2 = std::distance(middle2, middle3);
        DifferenceType a = 2 * s1 + n1;
        DifferenceType b = a + n1 + n2;

        power_type result = 0;

        while (1)
        {
            ++result;

            if (a >= n)
            {
                assert(b >= a);
                a -= n;
                b -= n;
            }
            else if (b >= n)
            {
                break;
            }

            assert(a < b && b < n);
        
            a <<= (DifferenceType)1;
            b <<= (DifferenceType)1;
        }

        return result;
    }

    // https://github.com/sebawild/nearly-optimal-mergesort-code/tree/master
    template <typename I>
        requires (Policy == power_policy::java)
    static constexpr power_type node_power(I first, I middle1, I middle2, I middle3, I last)
    {
        // run1 => [middle1, middle2)
        // run2 => [middle2, middle3)

        using DifferenceType = typename std::iterator_traits<I>::difference_type;

        DifferenceType twoN = std::distance(first, last) << 1;
        DifferenceType startA = std::distance(first, middle1);
        DifferenceType startB = std::distance(first, middle2);
        DifferenceType endB = std::distance(first, middle3);

        DifferenceType l = startA + startB;
        DifferenceType r = startB + endB + 1;

        power_type a = (power_type)((l << (DifferenceType)31) / twoN);
        power_type b = (power_type)((r << (DifferenceType)31) / twoN);

        return std::countl_zero(a ^ b); // count leading zeros
    }

    template <typename I, typename Comp>
    static constexpr I extend_run_right(I first, I last, Comp comp) 
    {
        [[assume(first != last)]];
        
        I second = count_run_and_make_ascending(first, last, comp);

        // We use a fixed minimum run length to ensure that the insertion sort described in the paper.
        // The Python implementation uses min_run_length(which used in TimSort) to determine the minimum run length.
        if (second - first < MinRunLen)
        {
            auto third = std::ranges::next(first, MinRunLen, last);
            insertion_sort_rest(first, second, third, comp);
            second = third;
        }

        return second;
    }

    template <typename I, typename Comp>
    static constexpr void merge_at(std::vector<I>& runs, typename std::vector<I>::size_type n, Comp comp)
    {
        std::inplace_merge(runs[n].first, runs[n + 1].first, runs[n + 2].first, comp);
        runs.erase(runs.begin() + n + 1);
    }

    template <typename I, typename Comp>
        requires (Policy == power_policy::java)
    static constexpr void merge_force_collapse(std::vector<I>& runs, Comp comp)
    {
        auto last = runs.back().first;
        runs.pop_back();

        // Different from TimSort, we merge all runs in reverse order described in the paper.
        for (auto i = runs.size(); i >= 1; --i)
        {
            std::inplace_merge(runs[i - 1].first, runs[i].first, last, comp);
        }
    }

    template <typename I, typename Comp>
        requires (Policy == power_policy::python)
    static constexpr void merge_force_collapse(std::vector<I>& runs, Comp comp)
    {
        if (runs.size() <= 2)
        {
            return; // nothing to collapse
        }

        while (runs.size() > 2)
        {
            size_t n = runs.size() - 3; // Top

            if (n > 0 && std::distance(runs[n - 1].first, runs[n].first) < std::distance(runs[n + 1].first, runs[n + 2].first))
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
        if (last - first < MinRunLen) 
        {
            insertion_sort(first, last, comp);
            return;
        }

        using power_run = std::pair<I, power_type>;

        const auto n = std::distance(first, last);
        std::vector<power_run> runs;
        
        runs.reserve(std::log2(n) + 1); // reserve enough space for runs
        runs.emplace_back(first, 0);
        runs.emplace_back(extend_run_right(first, last, comp), 0);

        auto left = runs.back().first;

        while (left != last)
        {
            auto right = extend_run_right(left, last, comp);

            // Power
            // Stack => [iter1, iter2, ...
            // interval1 = [iter1, iter2)
            // interval2 = [iter2, right)
            auto power = node_power(first, (runs.end() - 2)->first, (runs.end() - 1)->first, right, last);

            // The runs cannot be empty since the power of first two elements are zero,
            // and the loop will stop when the second run is reached. 
            while (runs.back().second > power)
            {
                std::inplace_merge((runs.end() - 2)->first, (runs.end() - 1)->first, right, comp);
                runs.pop_back();
            }

            runs.emplace_back(right, power);
            left = right;
        }
        
        merge_force_collapse(runs, comp);
    }
};

} // namespace cpp::ranges::detail

namespace cpp::ranges
{

inline constexpr detail::sorter<detail::power_sorter<>> power_sort;

}
