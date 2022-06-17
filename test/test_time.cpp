#define CATCH_CONFIG_MAIN


#include "test_time.hpp"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>

#include <lv_cpp/collections/sorted_list.hpp>
#include <lv_cpp/collections/skip_list.hpp>
#include <lv_cpp/collections/hash_table.hpp>
// #include <lv_cpp/collections/internal/raw_hash_table.hpp>


using SET1 = std::set<int>;
using SET2 = leviathan::collections::sorted_list<int>;
using SET3 = leviathan::collections::skip_list<int>;

using UNORDERED_SET1 = std::unordered_set<int>;
using UNORDERED_SET2 = std::unordered_set<int>;
using UNORDERED_SET3 = ::leviathan::collections::hash_table<int>;

TEST_CASE("duplicate_ordered_collections_random_insert")
{
    using namespace leviathan::test;

    BENCHMARK("std::set")
    {
        return random_insert_test<SET1>();
    };

    BENCHMARK("sorted_list")
    {
        return random_insert_test<SET2>();
    };

    BENCHMARK("skip_list")
    {
        return random_insert_test<SET3>();
    };
}

TEST_CASE("duplicate_ordered_collections_ascending_insert")
{
    using namespace leviathan::test;

    BENCHMARK("std::set")
    {
        return ascending_insert_test<SET1>();
    };

    BENCHMARK("sorted_list")
    {
        return ascending_insert_test<SET2>();
    };

    BENCHMARK("skip_list")
    {
        return ascending_insert_test<SET3>();
    };
}

TEST_CASE("duplicate_ordered_collections_descending_insert")
{
    using namespace leviathan::test;

    BENCHMARK("std::set")
    {
        return descending_insert_test<SET1>();
    };

    BENCHMARK("sorted_list")
    {
        return descending_insert_test<SET2>();
    };

    BENCHMARK("skip_list")
    {
        return descending_insert_test<SET3>();
    };
}

TEST_CASE("duplicate_unordered_search")
{
    using namespace leviathan::test;
    UNORDERED_SET1 s1;
    UNORDERED_SET2 s2;
    UNORDERED_SET3 s3;

    random_insert(s1, s2, s3);
    
    BENCHMARK("std::unordered_set")
    {
        return search_test(s1);
    };

    BENCHMARK("hash_table_variant")
    {
        return search_test(s2);
    };

    BENCHMARK("hash_table2")
    {
        return search_test(s3);
    };

}

TEST_CASE("duplicate_unordered_remove")
{
    using namespace leviathan::test;
    UNORDERED_SET1 s1;
    UNORDERED_SET2 s2;
    UNORDERED_SET3 s3;

    random_insert(s1, s2, s3);
    
    BENCHMARK("std::unordered_set")
    {
        return remove_test(s1);
    };

    BENCHMARK("hash_table_variant")
    {
        return remove_test(s2);
    };

    BENCHMARK("hash_table2")
    {
        return remove_test(s3);
    };

}



