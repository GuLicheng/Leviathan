
#include <catch2/catch_all.hpp>

#include "tim_sort.hpp"
#include "power_sort.hpp"
#include "intro_sort.hpp"
#include "pdq_sort.hpp"

template <typename RandomAccessRange, typename Sorter, typename Compare = std::ranges::less>
bool CheckIsOrdered(RandomAccessRange rg, Sorter sorter, Compare compare = {})
{
    sorter(rg, compare);
    return std::ranges::is_sorted(rg, compare);
}

template <typename Sorter>
void TestAscending()
{
    std::vector ascending(std::from_range, std::views::iota(1, 1000));
    CHECK(CheckIsOrdered(std::move(ascending), Sorter()));
}

template <typename Sorter>
void TestDescending()
{
    std::vector descending(std::from_range, std::views::iota(1, 1000) | std::views::reverse);
    CHECK(CheckIsOrdered(std::move(descending), Sorter()));
}

template <typename Sorter>
void PipeLine()
{
    std::vector pipeline_numbers(std::from_range, std::views::iota(1, 1000));
    std::ranges::reverse(pipeline_numbers | std::views::drop(500));
    CHECK(CheckIsOrdered(std::move(pipeline_numbers), Sorter()));
}

template <typename Sorter>
void TestRandom()
{
    std::vector<int> random_numbers(1000);
    std::generate(random_numbers.begin(), random_numbers.end(), std::mt19937{std::random_device{}()});
    CHECK(CheckIsOrdered(std::move(random_numbers), Sorter()));
}

template <typename Sorter>
void TestRandomString()
{
    std::mt19937 rd(0);
    std::vector<std::string> random_strings(num);
    std::generate(random_strings.begin(), random_strings.end(), [&]() {
        std::string str(rd() % str_max_length, ' ');
        std::generate(str.begin(), str.end(), [&]() { return 'a' + rd() % 26; });
        return str;
    });
    CHECK(CheckIsOrdered(std::move(random_strings), Sorter()));
}

template <typename Sorter>
void TestRandomReverse()
{
    std::vector<int> random_numbers(1000);
    std::generate(random_numbers.begin(), random_numbers.end(), std::mt19937{std::random_device{}()});
    CHECK(CheckIsOrdered(std::move(random_numbers), Sorter(), std::ranges::greater()));
}














