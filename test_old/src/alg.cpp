#include <iostream>
#include <vector>
#include <random>
#include <leviathan/algorithm/sort.hpp>
#include <algorithm>
#include <ranges>
#include <assert.h>

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
#include <leviathan/meta/template_info.hpp>
#include <leviathan/algorithm/pdqsort.h>
#include <leviathan/algorithm/sort.hpp>
#include <leviathan/algorithm/TimSort.h>

auto random = []() {
    static std::random_device rd;
    return rd() % 1000'0000;
};

template <typename Sort>
void test(Sort s, const std::vector<int>& vec, std::string_view name)
{
    auto vec2 = vec;

    // less relationship
    s(vec2);
    assert(std::ranges::is_sorted(vec2));
    
    // greater relationship
    s(vec2, std::ranges::greater{});
    std::ranges::reverse(vec2);
    // assert(std::ranges::is_sorted(vec2 | std::views::reverse));
    assert(std::ranges::is_sorted(vec2));

    // test for stability
    std::vector<std::pair<int, int>> vec3;
    std::ranges::transform(vec, std::back_inserter(vec3), [cnt = 0](int x) mutable
    {
        return std::make_pair(x, cnt++);
    });
    s(vec3, {}, &std::pair<int, int>::first);

    auto iter = std::ranges::adjacent_find(vec3, [](const auto left, const auto right)
    {
        return left.first == right.first && left.second > right.second;
    });
    std::cout << name << " is stable sort? " << std::boolalpha << (iter == vec3.end()) << '\n';    
}

void test_stability()
{
    std::vector<int> vec;
    std::generate_n(std::back_inserter(vec), 10000, random);

    test(leviathan::insertion_sort, vec, "insertion sort");
    test(leviathan::merge_sort, vec, "merge sort");
    test(leviathan::heap_sort, vec, "heap sort");
    test(leviathan::tim_sort, vec, "tim sort");
    test(leviathan::quick_sort, vec, "quick sort");
    test(leviathan::intro_sort_recursive, vec, "intro_sort_recursive");
    test(leviathan::intro_sort_iteration, vec, "intro_sort_iteration");
}


template <typename T>
void PrintVector(const std::vector<T>& v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>{std::cout, "\n"});
}

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

    int m_num = 1e8;
    int m_max = 100;

    static void PrintVec(const std::vector<int>& v)
    {
        for (auto i : v) stream << i << ' ';
        stream << '\n';
    }

    std::vector<std::string> ReadContext(const char* file = "a.txt")
    {
        std::fstream fs{file};
        if (!fs) return { };
        std::vector<std::string> ret;
        std::copy(std::istream_iterator<std::string>{fs}, std::istream_iterator<std::string>{}, std::back_inserter(ret));
        return ret;
    }

    std::vector<int> RandomRangeInt()
    {
        auto random_generator = [&]() {
            return rd();
        };
        std::vector<int> ret;
        std::generate_n(std::back_inserter(ret), m_num, random_generator);
        return ret;
    }

    std::vector<int> RandomAscending()
    {
        std::vector<int> v; v.reserve(m_num);
        for (int i = 0; i < m_num; ++i) v.push_back(i);
        return v;
    }

    std::vector<int> AllEqualZero()
    {
        return std::vector<int>(m_num, 0);
    }

    std::vector<int> RandomDescending()
    {
        std::vector<int> v; v.reserve(m_num);
        for (int i = 0; i < m_num; ++i) v.push_back(-i);
        return v;
    }

    std::vector<int> PipeOrgan() 
    {
        std::vector<int> v; v.reserve(m_num);
        for (int i = 0; i < m_num/2; ++i) v.push_back(i);
        for (int i = m_num/2; i < m_num; ++i) v.push_back(m_num - i);
        return v;
    }

    std::vector<int> PushFrontInt() 
    {
        std::vector<int> v; v.reserve(m_num);
        for (int i = 1; i < m_num; ++i) v.push_back(i);
        v.push_back(0);
        return v;
    }

    std::vector<int> push_middle_int() 
    {
        std::vector<int> v; v.reserve(m_num);
        for (int i = 0; i < m_num; ++i) {
            if (i != m_num/2) v.push_back(i);
        }
        v.push_back(m_num/2);
        return v;
    }

};

struct Recorder
{
    template <typename Sorter, typename T>
    static auto TestSort(Sorter sort_fn, std::vector<T> v, std::string_view sort_name)
    {
        auto tp1 = std::chrono::high_resolution_clock::now();
        sort_fn(v.begin(), v.end(), std::less<>{});
        auto tp2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> cost = tp2 - tp1;
        assert(std::is_sorted(v.begin(), v.end()));
        return SortInfo{
            .m_sort_name = sort_name,
            .m_cost_time = cost.count() * 1000 // ms
        };
    }
};

template <typename... Sorters>
struct SortAlgorithmSet
{
    SortAlgorithmSet(Sorters...) { } // just for deducing type

    inline static RandomRange rg{ .m_num = (int)3e6, .m_max = -1 };

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

    SortAlgorithmSet& TestString()
    {
        std::vector<SortInfo> vec;
        auto data = rg.ReadContext();
        stream << "[Context]\n";
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

void test_speed()
{
    SortAlgorithmSet s{
        std::ranges::sort,
        leviathan::intro_sort_iteration,
        leviathan::intro_sort_recursive,
    };
    s
    // .TestString()
    // .TestAllEqualZero()
    // .TestRandomAscending()
    // .TestRandomDescending()
    .TestRandomInt()
    ;
}


int main(int argc, char const *argv[])
{
    // test_stability();
    test_speed();    
    std::cout << "OK\n";
    return 0;
}
