#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <tuple>
#include <ranges>
#include <assert.h>
#include <string>

#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/algorithm/pdqsort.h>
#include <lv_cpp/algorithm/sort.hpp>
#include <lv_cpp/algorithm/TimSort.h>

struct SortInfo
{
    std::string_view m_sort_name;
    double m_cost_time;

    friend std::ostream& operator<<(std::ostream& os, const SortInfo& self)
    {
        std::string info;
        info += std::to_string(self.m_cost_time);
        info += ' ';
        info += self.m_sort_name;
        return os << info << '\n';
    }
};

struct RandomRange
{
    inline static std::random_device rd;

    int m_num = 1000'000;
    int m_max = 100;

    static void PrintVec(const std::vector<int>& v)
    {
        for (auto i : v) std::cout << i << ' ';
        std::cout << '\n';
    }

    std::vector<int> RandomRangeInt()
    {
        auto random_generator = [&]() {
            return rd() % m_max;
        };
        std::mt19937_64 e{ rd() };
        std::vector<int> ret;
        std::generate_n(std::back_inserter(ret), m_num, e);
        return ret;
    }

    std::vector<int> RandomAscending()
    {
        auto ret = RandomRangeInt();
        std::sort(ret.begin(), ret.end());
        return ret;
    }

    std::vector<int> AllEqualZero()
    {
        return std::vector<int>(m_num, 0);
    }

    std::vector<int> RandomDescending()
    {
        auto ret = RandomRangeInt();
        std::sort(ret.begin(), ret.end(), std::greater<>{});
        return ret;
    }

    std::vector<int> RandomPipeOrganInt()
    {
        auto ret = RandomRangeInt();
        auto iter = ret.begin() + ret.size() / 2;
        std::copy(ret.begin(), iter, ret.rbegin());
        return ret;
    }

};

struct Recorder
{
    template <typename Sorter>
    static auto TestSort(Sorter sort_fn, std::vector<int> v, std::string_view sort_name)
    {
        auto tp1 = std::chrono::high_resolution_clock::now();
        sort_fn(v.data(), v.data() + v.size(), std::less<>{});
        auto tp2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> cost = tp2 - tp1;
        return SortInfo{
            .m_sort_name = sort_name,
            .m_cost_time = cost.count() * 1000 // ms
        };
    }
};

template <typename... Sorters>
struct SortAlgorithmSet
{
    SortAlgorithmSet(Sorters...) { } // just for deduce type

    inline static RandomRange rg{ .m_num = (int)1e6, .m_max = 100 };

    SortAlgorithmSet& TestRandomInt()
    {
        std::vector<SortInfo> vec;
        auto data = rg.RandomRangeInt();
        std::cout << "=========================Random Int=========================\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{std::cout});
        std::cout << "=============================================================\n";
        return *this;
    }

    SortAlgorithmSet& TestRandomAscending()
    {
        std::vector<SortInfo> vec;
        auto data = rg.RandomAscending();
        std::cout << "====================Random Ascending=========================\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{std::cout});
        std::cout << "=============================================================\n";
        return *this;
    }

    SortAlgorithmSet& TestRandomDescending()
    {
        std::vector<SortInfo> vec;
        auto data = rg.RandomDescending();
        std::cout << "====================Random Descending=========================\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{std::cout});
        std::cout << "=============================================================\n";
        return *this;
    }

    SortAlgorithmSet& TestRandomPipeOrganInt()
    {
        std::vector<SortInfo> vec;
        auto data = rg.RandomPipeOrganInt();
        std::cout << "====================RandomPipeOrganInt=========================\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{std::cout});
        std::cout << "=============================================================\n";
        return *this;
    }

    SortAlgorithmSet& TestAllEqualZero()
    {
        std::vector<SortInfo> vec;
        auto data = rg.AllEqualZero();
        std::cout << "====================AllEqualZero=========================\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{std::cout});
        std::cout << "=============================================================\n";
        return *this;
    }

};

int main()
{
    SortAlgorithmSet s{
        std::ranges::sort, 
        std::ranges::stable_sort,
        leviathan::tim_sort_fn{},
        gfx::timsort_fn{},
        pdqsort_fn{},
        pdqsort_branchless_fn{}
    };
    s.TestRandomInt()
    .TestAllEqualZero()
    .TestRandomPipeOrganInt()
    .TestRandomDescending()
    .TestRandomAscending();
    return 0;
}