/*
    This file is used for test hashtable and hash function
*/

#define CATCH_CONFIG_MAIN

#include "test_time.hpp"

#include <lv_cpp/collections/hash_table.hpp>
#include <lv_cpp/collections/hash_table2.hpp>


#include <string>
#include <unordered_map>
#include <unordered_set>


using hash1 = std::unordered_set<int>;
using hash2 = leviathan::collections::hash_table<int>;
using hash3 = leviathan::collections::hash_table2<int>;



TEST_CASE("duplicate_unordered_collections_random_insert")
{
    using namespace leviathan::test;

    BENCHMARK("std::unordered_set")
    {
        return random_insert_test<hash1>();
    };

    BENCHMARK("hash_table")
    {
        return random_insert_test<hash2>();
    };

    BENCHMARK("hash_table2")
    {
        return random_insert_test<hash3>();
    };

}
TEST_CASE("duplicate_unordered_collections_random_search")
{
    hash1 s1;
    hash2 s2;
    hash3 s3;

    using namespace leviathan::test;

    random_insert(s1, s2, s3);
    

    BENCHMARK("std::unordered_set")
    {
        return search_test(s1);
    };

    BENCHMARK("hash_table")
    {
        return search_test(s2);
    };

    BENCHMARK("hash_table2")
    {
        return search_test(s3);
    };

}

using map1 = std::unordered_map<int, std::string>;
using map2 = leviathan::collections::hash_map<int, std::string>;
using map3 = leviathan::collections::hash_map2<int, std::string>;

TEST_CASE("duplicate_unordered_map_collections_random_search")
{
    map1 s1;
    map2 s2;
    map3 s3;

    using namespace leviathan::test;

    random_insert_map(s1, s2, s3);

    BENCHMARK("std::unordered_map")
    {
        return search_test(s1);
    };

    BENCHMARK("hash_map")
    {
        return search_test(s2);
    };

    BENCHMARK("hash_map2")
    {
        return search_test(s3);
    };


    std::cout << s1.size() << '\n';
    std::cout << s2.size() << '\n';
    std::cout << s3.size() << '\n';
}








