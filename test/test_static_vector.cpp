#include <leviathan/collections/static_vector.hpp>
#include <catch2/catch_all.hpp>

using leviathan::collections::static_vector;

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


