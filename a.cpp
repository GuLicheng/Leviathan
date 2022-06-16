#include <cstddef>
#include <type_traits>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <random>
#include <lv_cpp/collections/internal/raw_hash_table.hpp>
#include <lv_cpp/utils/timer.hpp>

template <typename C>
void try_insert(C& c, int value)
{
    auto ret = c.emplace(value);
    std::cout << ret.first.m_idx << '-' << ret.second << '\n';
}

template <typename C>
void try_erase(C& c, int value)
{
    auto ret = c.erase(value);
    std::cout << "Remove " << ret << " elements.\n";
}

template <typename C>
void print_range(C& c)
{
    for (auto val : c)
        std::cout << val << ' ';
    std::endl(std::cout);
}

int main() 
{
    using T = ::leviathan::collections::hash_set<int>;
    static_assert(std::ranges::forward_range<T>);
    T hs;
    char arr[10];
    std::vector values = { 0, 3, -1, 3, 2, 3, 4 };

    std::random_device rd;

    std::generate_n(std::back_inserter(values), 100000, std::ref(rd));
    std::unordered_set<int> us;

    {
        ::leviathan::timer _{"Insert"};
        for (auto val : values) hs.insert(val);
    }

    {
        ::leviathan::timer _{"STL"};
        for (auto val : values) us.insert(val);
    }

    std::cout << "Distance = " << std::ranges::distance(hs.begin(), hs.end()) << '\n';
    std::cout << "Size = " << hs.size() << '\n';
    std::cout << "Size = " << us.size() << '\n';
    std::cout << "Ok\n";
}





