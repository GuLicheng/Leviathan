#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <tuple>
#include <ranges>
#include <assert.h>
#include <string>
#include <fstream>
#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/algorithm/pdqsort.h>
#include <lv_cpp/algorithm/sort.hpp>
#include <lv_cpp/algorithm/TimSort.h>

struct SortInfo
{
    std::string_view m_sort_name;
    double m_cost_time;

    std::string_view ExtraceSortName() const
    {
        auto idx1 = m_sort_name.find_last_of(":");
        idx1++;
        auto idx2 = m_sort_name.find_last_of("_fn");
        return m_sort_name.substr(idx1, idx2 - idx1 - 2);
    }

    friend std::ostream& operator<<(std::ostream& os, const SortInfo& self)
    {
        std::string info;
        info += self.ExtraceSortName();
        info += "=";
        info += std::to_string(self.m_cost_time);
        return os << info << '\n';
    }
};

std::ostream& stream = std::cout;

struct RandomRange
{
    inline static std::random_device rd;

    int m_num = 1000'000;
    int m_max = 100;

    static void PrintVec(const std::vector<int>& v)
    {
        for (auto i : v) stream << i << ' ';
        stream << '\n';
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
        stream << "[Random Int]\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{stream});
        stream << "\n";
        return *this;
    }

    SortAlgorithmSet& TestRandomAscending()
    {
        std::vector<SortInfo> vec;
        auto data = rg.RandomAscending();
        stream << "[Random Ascending]\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{stream});
        stream << "\n";
        return *this;
    }

    SortAlgorithmSet& TestRandomDescending()
    {
        std::vector<SortInfo> vec;
        auto data = rg.RandomDescending();
        stream << "[Random Descending]\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{stream});
        stream << "\n";
        return *this;
    }

    SortAlgorithmSet& TestAllEqualZero()
    {
        std::vector<SortInfo> vec;
        auto data = rg.AllEqualZero();
        stream << "[AllEqualZero]\n";
        (vec.push_back(Recorder::TestSort(Sorters{}, data, TypeInfo(Sorters))), ...);
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<SortInfo>{stream});
        stream << "\n";
        return *this;
    }

};

int main(int argc, const char* argv[])
{

    SortAlgorithmSet s{
        std::ranges::sort, 
        std::ranges::stable_sort,
        leviathan::tim_sort_fn{},
        gfx::timsort_fn{},
        PDQSort_fn{},
        PDQSortBranchless_fn{},
        leviathan::quick_sort,
        leviathan::intro_sort
    };
    s.TestRandomInt()
    .TestAllEqualZero()
    .TestRandomDescending()
    .TestRandomAscending();
    return 0;
}