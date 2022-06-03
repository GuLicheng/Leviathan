#pragma once

#include <algorithm>
#include <functional>

namespace leviathan
{
    template <typename I, typename Comp = std::less<>>
    constexpr bool combination(I first1, I middle, I last2, Comp comp = {})
    {

        auto ori_f = first1, ori_l = last2;

        auto last1 = middle, first2 = middle;

        if (first1 == last1 || first2 == last2)
            return false;

        I m1 = last1;
        I m2 = last2;
        --m2;

        // find first element less than last element
        while (--m1 != first1 && !comp(*m1, *m2))
            ;

        bool result = m1 == first1 && !comp(*first1, *m2);
        if (!result)
        {
            while (first2 != m2 && !comp(*m1, *first2))
                ++first2;

            first1 = m1;
            std::iter_swap(first1, first2);
            ++first1;
            ++first2;
        }

        if (first1 != last1 && first2 != last2)
        {
            m1 = last1;
            m2 = first2;

            while (m1 != first1 && m2 != last2)
            {
                std::iter_swap(--m1, m2);
                ++m2;
            }

            std::reverse(first1, m1);
            std::reverse(first1, last1);
            std::reverse(m2, last2);
            std::reverse(first2, last2);
        }

        return !result;
    }

    template <typename I>
    constexpr bool next_combination(I first, I middle, I last)
    {
        return combination(first, middle, last);
    }

    template <typename I>
    constexpr bool prev_combination(I first, I middle, I last)
    {
        return combination(first, middle, last, std::greater<>());
    }

}
