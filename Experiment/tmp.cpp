#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <ranges>
/*
   index 1, 2, 3, 4
         4
*/

template <typename ContiguousIterator>
std::vector<int> count_region(ContiguousIterator superpixel, ContiguousIterator label, int len, int pixelnum, int unlabeled = 255)
{
    std::vector<std::unordered_map<int, int>> table;
    table.resize(pixelnum);

    for (int i = 0; i < len; ++i)
    {
        auto superpixel_id = superpixel[i];
        auto label_id = label[i];
        if (label_id != 255)
            table[superpixel_id][label_id]++;
    }

    std::vector<int> result(pixelnum, unlabeled);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        auto iter = std::max_element(table[i].begin(), table[i].end(), [](const auto& x, const auto& y) {
            return x.second < y.second;
            });
        if (iter != table[i].end())
            result[i] = iter->first;
    }
    return result;
}

template <typename ContiguousIterator>
void superpixel_fill(ContiguousIterator superpixel, ContiguousIterator dest, int len, const std::vector<int>& region)
{
    for (int i = 0; i < len; ++i)
    {
        auto superpixel_id = superpixel[i];
        auto class_id = region[superpixel_id];
        dest[i] = class_id;
    }
}

template <typename ContiguousIterator>
void super_pixel_expand(
    ContiguousIterator superpixel,
    ContiguousIterator label,
    ContiguousIterator dest,
    int length,
    int superpixel_num)
{
    auto region = count_region(superpixel, label, length, superpixel_num);
    superpixel_fill(superpixel, dest, length, region);
}


int main()
{

    std::vector<int> superpixel = {
        0, 0, 0, 1, 1
    };

    std::vector<int> label = {
        255, 255, 255, 0, 0
    };

    std::vector<int> result(5, -1);

    super_pixel_expand(superpixel.data(), label.data(), result.data(), superpixel.size(), 2);

    for (auto x : result)
        std::cout << x << ' ';

    auto rg = std::views::iota(1, 10)
        | std::views::reverse
        | std::views::transform([](int x) { return x + 1; })
        | std::views::filter([](int x) { return x & 1; });

    std::ranges::copy(rg, std::ostream_iterator<int>{std::cout, " "});

}




