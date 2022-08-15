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





// #include <assert.h>

// int main()
// {
//     std::vector values = { 1, 2, 3, 4, 5 };
//     std::vector<int> dest;
//     pair_wise_transform(values.begin(), values.end(), std::back_inserter(dest), std::plus<>());
//     for (auto val : dest)
//         std::cout << val << '\n';
// }

