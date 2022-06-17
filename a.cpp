#include <cstddef>
#include <type_traits>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <random>
#include <lv_cpp/collections/internal/raw_hash_table.hpp>
#include <lv_cpp/collections/hash_table.hpp>
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

inline std::random_device rd;
#define DEFAULT_NUM 1'000'000
inline constexpr auto default_num = DEFAULT_NUM; // DEFAULT_NUM x Catch::DataConfig::benchmarkSamples

std::vector<int> random_range_int(int n = default_num)
{
    std::uniform_int_distribution<int> rd_gen(INT_MIN, INT_MAX);
    std::mt19937 gen(rd());

    auto random_generator = [&]() {
        return (rd() % DEFAULT_NUM) << 7;
        return (rd() % DEFAULT_NUM);
    };
    std::vector<int> ret;
    ret.reserve(n);
    std::generate_n(std::back_inserter(ret), n, random_generator);
    return ret;
}

int main() 
{
    using T = ::leviathan::collections::hash_set<int>;
    using U = ::leviathan::collections2::hash_table<int>;
    static_assert(std::ranges::forward_range<T>);
    T hs;
    U ohs;


    std::vector values = random_range_int();
    std::vector finds = random_range_int();

    std::unordered_set<int> us;
    int res1 = 0, res2 = 0;
    {
        ::leviathan::timer _{"STL"};
        for (auto val : values) us.insert(val);
        // for (auto val : finds) res2 += us.contains(val);
    }

    {
        ::leviathan::timer _{"Insert"};
        for (auto val : values) hs.insert(val);
        // for (auto val : finds) res1 += hs.contains(val);
    }

    {
        ::leviathan::timer _{"Old hash table"};
        for (auto val : values) ohs.insert(val);
    }

    std::cout << res1 << '\n';
    std::cout << res2 << '\n';
    std::cout << hs.load_factor() << '\n';
    std::cout << us.load_factor() << '\n';

    std::cout << T::policy_type::generator_type::count << '\n';
    std::cout << U::policy_type::count << '\n';

    std::cout << "Ok\n";
}





