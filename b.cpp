#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

template <typename I, typename Comp = std::less<>>
std::pair<I, I> min_and_second_min_element(I first, I last, Comp comp = {})
{
    auto forward = std::ranges::next(first, 1, last);
    std::pair<I, I> result { first, forward };

    if (forward == last)
        return result;

    if (!comp(result.first, result.second))
        std::swap(result.first, result.second);
    
    for (; forward != last; ++forward) 
    {
        // result.first < result.second 
        if (comp(*forward, *result.first))
        {
            result.second = result.first;
            result.first = forward;
        }
        else if (comp(*forward, *result.second))
            result.second = forward;
    }

    return result;

}


int main()
{
    std::vector<int> values = { 0, 8, 2, -3, 4 };
    auto [min1, min2] = min_and_second_min_element(values.begin(), values.end());
    std::cout << "min = " << *min1 << " and second min = " << *min2 << '\n';
}

