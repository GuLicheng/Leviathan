#pragma once


#include <map>
#include <set>
#include <random>
#include <ranges>
#include "base.hpp"

LV_TEST_BEGIN


// unique set
template <typename SetContainer>
void simple_unique_set_container_test()
{
    std::vector values = { 3, 5, 3, 3, 5, 6 };

    SetContainer c;

    for (auto val : values) c.insert(val);
    
    SECTION("Insert and size")
    {
        // if we insert at this scope, the c will be empty after exit this scope
        REQUIRE(c.size() == 3); // 3, 5, 6
    }

    SECTION("search elements")
    {
        REQUIRE(c.find(1) == c.end());
        REQUIRE(c.find(2) == c.end());
        REQUIRE(c.find(3) != c.end());
        REQUIRE(c.find(4) == c.end());
        REQUIRE(c.find(5) != c.end());
        REQUIRE(c.find(6) != c.end());
    }

    SECTION("remove element")
    {
        c.erase(1);
        REQUIRE(c.size() == 3);
        c.erase(3);
        REQUIRE(c.size() == 2);
        c.erase(5);
        c.erase(6);
        REQUIRE(c.empty());
    }

}

template <typename SetContainer>
void unique_set_search_method()
{
    std::vector values = { 1, 3, 5, 6 };

    SetContainer c;

    for (auto val : values) c.insert(val);

    SECTION("find")
    {
        REQUIRE(c.find(1) != c.end());
        REQUIRE(c.find(2) == c.end());
    }

    SECTION("lower_bound")
    {
        REQUIRE(c.lower_bound(1) == c.find(1));
        REQUIRE(c.lower_bound(2) == c.find(3));
        REQUIRE(c.lower_bound(9) == c.end());
    }

    SECTION("upper_bound")
    {
        REQUIRE(c.upper_bound(1) == c.find(3));
        REQUIRE(c.upper_bound(2) == c.find(3));
        REQUIRE(c.upper_bound(3) == c.find(5));
        REQUIRE(c.upper_bound(6) == c.end());
    }

    SECTION("equal_range")
    {
        auto [lower, upper] = c.equal_range(3);
        REQUIRE(lower == c.lower_bound(3));
        REQUIRE(upper == c.upper_bound(3));
    }

}

template <typename SetContainer>
void unique_set_insert_method()
{
    SetContainer c;

    SECTION("insert and it's return result")
    {
        REQUIRE(*c.insert(1).first == 1);
        REQUIRE(c.insert(1).second == false);
        REQUIRE(c.insert(2).second == true);
        REQUIRE(c.size() == 2);
    }

    SECTION("emplace and it's return result")
    {
        REQUIRE(*c.emplace(1).first == 1);
        REQUIRE(c.emplace(1).second == false);
        REQUIRE(c.emplace(2).second == true);
        REQUIRE(c.size() == 2);
    }

}

template <typename SetContainer>
void simple_unique_set_iterator_test()
{

    STATIC_REQUIRE(std::ranges::bidirectional_range<SetContainer>);

    SetContainer c;

    std::vector values = { 1, 2, 3, 4, 5 };

    std::ranges::copy(values, std::inserter(c, c.end()));

    REQUIRE(std::ranges::is_sorted(c));

    SECTION("remove elements by iterators")
    {
        c.clear();
        for (auto val : values) c.insert(val);
        auto iter = c.begin();
        while (iter != c.end())
        {
            iter = c.erase(iter);
        }
        REQUIRE(c.empty());
    }

    SECTION("reversed iterator")
    {
        REQUIRE(c.size() == values.size());
        auto reversed1 = c | std::views::reverse;
        auto reversed2 = values | std::views::reverse;
        auto is_equal = std::ranges::equal(reversed1, reversed2);
        REQUIRE(is_equal);
    }

    SECTION("remove elements by iterators2")
    {
        c.clear();
        for (auto val : values) c.insert(val);
        REQUIRE(c.erase(c.find(5)) == c.end());
        REQUIRE(c.erase(c.find(1)) != c.end());
    }

}

template <typename SetContainer>
void simple_unique_set_container_random_test()
{
    std::set<int> comparison;
    SetContainer c;
    static std::random_device rd;

    constexpr int N = 100'000;
    
    auto rand_seq = [](int n) {
        std::vector<int> vec;
        vec.reserve(n);
        std::generate_n(std::back_inserter(vec), n, std::ref(rd));
        return vec;
    };

    
    auto inserted_elements = rand_seq(N);
    auto found_elements = rand_seq(N);
    auto erased_elements = rand_seq(N);

    auto op = [&](auto& container) {
        std::ranges::copy(inserted_elements, std::inserter(container, container.end()));
        std::ranges::for_each(erased_elements, [&](const auto& value) {
            container.erase(value);
        });
        return std::ranges::count_if(found_elements, [&](const auto& value){
            return container.contains(value);
        });
    };

    CHECK(op(comparison) == op(c));

    auto is_equal = std::ranges::equal(comparison, c);
    REQUIRE(is_equal);
}


// unique map
template <typename MapContainer>
void simple_unique_map_container_test()
{
    MapContainer c;

    STATIC_REQUIRE(std::same_as<typename MapContainer::key_type, std::string>);
    STATIC_REQUIRE(std::same_as<typename MapContainer::mapped_type, int>);
    STATIC_REQUIRE(std::ranges::bidirectional_range<MapContainer>);

    c.insert({"Hello", 1});
    c.insert({"World", 2});
    c.insert({"!", 3});

    SECTION("operator[]")
    {
        c["Hello"] = 0;
        c["?"] = -1;
        REQUIRE(c.find("Hello")->second == 0);
        REQUIRE(c.find("?")->second == -1);
    }

    SECTION("find")
    {   
        REQUIRE(c.find("Hello") != c.end());
        REQUIRE(c.find("XXX") == c.end());
    }

    SECTION("remove elements")
    {
        c.erase("!");
        c.erase("XXX");
        REQUIRE(c.size() == 2);
    }

}





// multi-set


// multi-map



LV_TEST_END

