#include "sorted_list.hpp"
#include <set>
#include <vector>
#include <random>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <iterator>

using namespace leviathan::collections;

std::vector<int> random_range_int(int n = 100000)
{
    static std::random_device rd;
    auto random_generator = [&]() {
        return rd();
    };
    std::vector<int> ret;
    ret.reserve(n);
    std::generate_n(std::back_inserter(ret), n, random_generator);
    return ret;
}

void test()
{
    std::set<int> ss;
    sorted_set<int> s;

    auto numbers = random_range_int();
    for (auto number : numbers)
    {
        ss.insert(number);
        s.insert(number);
    }
    assert(std::ranges::equal(s, ss));

    std::cout << "Ok\n";
}

void print(std::vector<int>& v)
{
    for (const auto& value : v)
        std::cout << value << ' ';
    std::cout << '\n';
}


int main()
{
    std::vector<int> arr = { 1, 2, 3, 4, 5 };
    std::vector<int> diff;
    for (int i = 0; i < arr.size(); ++i)
    {
        if (i == 0)
            diff.emplace_back(arr[0]);
        else
            diff.emplace_back(arr[i] - arr[i - 1]);
    }


    print(arr);
    print(diff);


}
