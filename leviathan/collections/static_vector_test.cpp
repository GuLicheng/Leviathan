#include "static_vector.hpp"
#include <algorithm>
#include <catch2/catch_all.hpp>

using cpp::collections::static_vector;

TEST_CASE("ctor")
{
    static_vector<int, 24> v1;
    REQUIRE(v1.size() == 0);
    REQUIRE(v1.capacity() == 24);
    REQUIRE(v1.empty());
    REQUIRE(v1.max_size() == 24);


    static_vector<int, 8> v2(8);
    REQUIRE(v2.size() == v2.capacity());
    REQUIRE(v2.size() == 8);
    for (auto value : v2)
    {
        REQUIRE(value == 0);
    }

    static_vector<int, 8> v3(8, 1);
    REQUIRE(v3.size() == v3.capacity());
    REQUIRE(v3.size() == 8);
    for (auto value : v3)
    {
        REQUIRE(value == 1);
    }  

    std::initializer_list il = { 0, 1, 2, 3 };

    static_vector<int, 8> v5(il);
    REQUIRE(std::ranges::equal(il, v5));


    static_vector<int, 8> v6(2);
    v6 = v5;
    REQUIRE(std::ranges::equal(v6, v5));

    static_vector<int, 8> v7(2);
    v7 = v5;
    REQUIRE(std::ranges::equal(v7, v5));
}

TEST_CASE("insert and emplace")
{
    static_vector<int, 8> v1;
    v1.insert(v1.begin(), -1);
    REQUIRE(v1.size() == 1);
    REQUIRE(v1[0 == -1]);


    v1.emplace(v1.begin(), 0);
    v1.emplace_back(-2);

    REQUIRE(v1.size() == 3);
    REQUIRE(v1.front() == 0);
    REQUIRE(v1.back() == -2);

    v1.pop_back();
    v1.pop_back();
    v1.pop_back();

    REQUIRE(v1.empty());
}

TEST_CASE("erase")
{
    static_vector<int, 8> v1 = { 0, 1, 2, 3, 4, 5, 6, 7 };

    v1.erase(v1.begin(), v1.begin() + 5);

    v1.erase(--v1.end());

    REQUIRE(v1.size() == 2);
    REQUIRE(v1[0] == 5);
    REQUIRE(v1[1] == 6);

}

TEST_CASE("swap")
{
    static_vector<int, 8> v1, v2;

    SECTION("swap sequences with difference size")
    {

        v1.emplace_back(0);
        v1.emplace_back(1);
        v1.emplace_back(2);
        v1.emplace_back(3);

        v2.emplace_back(-1);
        v2.emplace_back(-2);

        v1.swap(v2);

        REQUIRE(v1.size() == 2);
        REQUIRE(v2.size() == 4);

        auto il1 = { 0, 1, 2, 3 }, il2 = { -1, -2 };
        REQUIRE(std::ranges::equal(v1, il2));
        REQUIRE(std::ranges::equal(v2, il1));
    }

    SECTION("swap sequences with same size")
    {
        v1.emplace_back(-1);
        v2.emplace_back(1);

        v1.swap(v2);

        REQUIRE(v1.size() == v2.size());
        REQUIRE(v1.front() == 1);
        REQUIRE(v2.front() == -1);
    }

}

TEST_CASE("compare")
{
    static_vector<int, 8> v1, v2, v3;

    v1.emplace_back(0);
    v1.emplace_back(1);
    v1.emplace_back(2);

    v2.emplace_back(0);
    v2.emplace_back(1);
    v2.emplace_back(2);

    v3.emplace_back(0);

    REQUIRE(v1 == v2);
    REQUIRE(v1 != v3);
    REQUIRE(v3 < v1);
    REQUIRE(v3 <= v1);
    REQUIRE(v1 > v3);
    REQUIRE(v1 >= v3);
}

