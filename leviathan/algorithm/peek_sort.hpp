// https://arxiv.org/pdf/1805.04154

// We may implement a peek sort algorithm based on the paper "Nearly Optimal Mergesort Code"
// if some language standard library use it as built-in sort algorithm.
#if 0
#pragma once

#include "basic_sort.hpp"
#include "tim_sort.hpp"

namespace cpp::ranges::detail 
{

template <int InsertionSortThreshold = 24> 
class peek_sorter : public tim_sorter<InsertionSortThreshold>
{
    using base = tim_sorter<InsertionSortThreshold>;

protected:

    template <typename I, typename Comp>
    static constexpr void sort_impl(I first1, I last1, I first2, I last2, Comp comp)
    {
        // [first1, last1) and [first2, last2) are sorted ranges
        // [last1, first2) is the range to be sorted

        if (first1 == first2 || last1 == last2) 
        {
            return; // nothing to sort
        }

        if (last2 - first1 <= InsertionSortThreshold) 
        {
            insertion_sort(last1, first2, comp);
            return;
        }

        I middle = first1 + (last1 - first1) / 2;

        if (middle <= last1)
        {
            // merge right part
            // sort_impl(last1, )
        }
        else if (middle >= first2)
        {

        }
        else
        {

        }
    }

public:

    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {

    }

};

}

#endif