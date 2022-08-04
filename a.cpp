#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>

class sorted_list_index_selector
{
public:

    template <typename I, typename Pred>
    void build(I first, I last, Pred pred)
    {
        std::inclusive_scan(first, last, std::back_inserter(m_index), [](const auto& x, const auto& y) {
            return x.size() + y.size();
        });
    }

    std::pair<std::size_t, std::size_t> index_of(std::size_t rank)
    {
        auto lower = std::lower_bound(m_index.begin(), m_index.end(), rank);
        if (lower == m_index.end())
            return { m_index.size(), 0 };
        if (*lower == rank) 
            return { std::distance(m_index.begin(), lower), *lower - *(lower - 1) };
        return { std::distance(m_index.begin(), lower), rank - *(lower - 1) };
    }

private:
    std::vector<std::size_t> m_index = { 0 };
};


/*
    https://en.cppreference.com/w/cpp/algorithm/ranges/transform
    Return:
        a unary_transform_result contains an input iterator equal to he last element not transformed
        and an output iterator to the element past the last element transformed.

    e.g.
        std::vector<int> v = { 0, 1, 2, 3, 4 }, dest;
        auto [ret, _] = pair_wise_transform(v.begin(), v.end(), std::back_inserter(dest), std::plus<>());
        assert(dest == { 1, 5 });
        assert(std::distance(ret, v.end()) == 1);
        assert(*ret == 4);
*/
template <std::input_iterator I, std::sentinel_for<I> S, std::weakly_incrementable O,
            std::copy_constructible F, typename Proj = std::identity>
requires std::indirectly_writable<O, std::indirect_result_t<F&, std::projected<I, Proj>, std::projected<I, Proj>>>
constexpr std::ranges::unary_transform_result<I, O> 
pair_wise_transform(I first, S last, O result, F op, Proj proj = {})
{
    auto second = std::ranges::next(first, 1, last);    

    // at least two elements
    for (; second != last;)
    {
        *result++ = std::invoke(op, std::invoke(proj, *first), std::invoke(proj, *second));
        std::ranges::advance(second, 2, last);
        std::ranges::advance(first, 2, last);
    }
    return { first, result };
}

#include <assert.h>

int main()
{
    std::vector<int> vec = { 1, 2, 3 }, res;
    std::inclusive_scan(vec.begin(), vec.end(), std::back_inserter(res));
    std::exclusive_scan(vec.begin(), vec.end(), std::back_inserter(res), 0);
    for (const auto& val : res)
        std::cout << val << ' ';
}

