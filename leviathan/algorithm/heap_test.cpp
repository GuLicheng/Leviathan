#include "heap.hpp"

#include <catch2/catch_all.hpp>
#include <vector>
#include <algorithm>
#include <random>

using BinaryMaxHeapFunction = leviathan::algorithm::nd_heap_fn<2>;
using QuaternaryMaxHeapFunction = leviathan::algorithm::nd_heap_fn<4>;

std::random_device rd;

auto RandomSequence(int count)
{
    std::vector<int> v(count);
    std::generate_n(v.begin(), count, std::mt19937(rd()));

    std::vector<std::string> strs;
    strs.reserve(count);

    for (auto value : v)
    {
        auto s = std::to_string(value);
        strs.emplace_back(s + s + s);
    }

    return strs;
}

bool CheckStringIsNotEmpty(const auto& sequence)
{
    return std::ranges::all_of(sequence, [](const auto& s) { return !s.empty(); });
}

auto GetRandomElement()
{
    auto s = std::to_string(rd());
    return s + s + s;
}

TEST_CASE("binary make heap")
{
    auto v = RandomSequence(1000);
    BinaryMaxHeapFunction::make_heap(v);
    CHECK(std::is_heap(v.begin(), v.end()));
    CHECK(BinaryMaxHeapFunction::is_heap(v));
    CHECK(CheckStringIsNotEmpty(v));
}

TEST_CASE("binary push heap")
{
    auto v = RandomSequence(1000);
    BinaryMaxHeapFunction::make_heap(v);

    for (int i = 0; i < 200; ++i)
    {
        v.emplace_back(GetRandomElement());
        BinaryMaxHeapFunction::push_heap(v);
        CHECK(std::is_heap(v.begin(), v.end()));
        CHECK(BinaryMaxHeapFunction::is_heap(v));
        CHECK(CheckStringIsNotEmpty(v));
    }
}

TEST_CASE("binary pop heap")
{
    auto v = RandomSequence(1000);
    BinaryMaxHeapFunction::make_heap(v);

    while (v.size())
    {
        BinaryMaxHeapFunction::pop_heap(v);
        v.pop_back();
        CHECK(std::is_heap(v.begin(), v.end()));
        CHECK(BinaryMaxHeapFunction::is_heap(v));
        CHECK(CheckStringIsNotEmpty(v));

    }
}

