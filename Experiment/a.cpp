#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>
#include <iostream>

#include <lv_cpp/math/algorithm.hpp>

struct merge_sort_fn
{
    template <std::random_access_iterator I, std::sentinel_for<I> S, 
        typename Comp = std::ranges::less, typename Proj = std::identity>
    requires std::sortable<I, Comp, Proj>
    constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
    {
        if (last - first > 1)
        {
            auto middle = first + (last - first) / 2;
            (*this)(first, middle, std::ref(comp), std::ref(proj));
            (*this)(middle, last, std::ref(comp), std::ref(proj));
            std::ranges::inplace_merge(first, middle, last, std::ref(comp), std::ref(proj));
        }
        return first;
    }

    template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity>
    requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj>
    constexpr std::ranges::borrowed_iterator_t<Range> operator()(Range&& r, Comp comp = {}, Proj proj = {}) const
    {
        return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
    }
};

inline constexpr merge_sort_fn merge_sort{ };

int main()
{
    std::vector arr{4, 5, 6, 1, 2, 3};
    merge_sort(arr);
    std::ranges::copy(arr, std::ostream_iterator<int>{std::cout, " "});
}

