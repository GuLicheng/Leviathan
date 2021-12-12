#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>
#include <iostream>
#include <functional>

std::vector arr{1, 8, 5, 4, 3, 2, 6, 8, 8, 8, 17};


struct insertion_sort_fn
{
    // for simplifier, use random_access_iterator 
    template <std::random_access_iterator I, std::sentinel_for<I> S, 
                typename Comp = std::ranges::less, typename Proj = std::identity>
    requires std::sortable<I, Comp, Proj>
    constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
    {
        if (first == last)
            return first;

        auto i = first + 1;
        for (; i != last; ++i)
        {
            auto j = i - 1;
            // if arr[j] <= arr[i] continue
            if (!std::invoke(comp, std::invoke(proj, *i), std::invoke(proj, *j))) continue;
            else
            {
                auto pos = std::ranges::upper_bound(first, i, *i, comp, proj);
                auto tmp = std::move(*i);
                std::ranges::move(pos, i, pos + 1);
                *pos = std::move(tmp);
            } 
        }
        return i;
    }

    template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity>
    requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj>
    constexpr std::ranges::borrowed_iterator_t<Range>
    operator()(Range&& r, Comp comp = {}, Proj proj = {}) const
    {
        return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }
};

inline constexpr insertion_sort_fn insert_sort{ };


int main()
{
    insert_sort(arr);
    // std::ranges::copy(arr, std::ostream_iterator<int>{std::cout, " "});
}

