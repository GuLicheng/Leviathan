#define CATCH_CONFIG_MAIN


#include "test_time.hpp"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>

#include <lv_cpp/collections/sorted_list.hpp>
#include <lv_cpp/collections/skip_list.hpp>
#include <lv_cpp/collections/hash_table.hpp>
#include <lv_cpp/collections/hash_table2.hpp>

// #include <lv_cpp/collections/bilibili.hpp>

using SET1 = std::set<int>;
using SET2 = leviathan::collections::sorted_list<int>;
using SET3 = leviathan::collections::skip_list<int>;

using UNORDERED_SET1 = std::unordered_set<int>;
using UNORDERED_SET2 = leviathan::collections::hash_table<int>;
using UNORDERED_SET3 = leviathan::collections::hash_table_impl<
        int, 
        std::hash<int>, 
        std::equal_to<>, 
        std::allocator<int>, 
        leviathan::collections::hash_set_config<int, std::hash<int>, std::equal_to<>, std::allocator<int>>, 
        leviathan::collections::detail::quadratic_policy<>, 
        true, 
        true, 
        17>;
using UNORDERED_SET4 = leviathan::collections::hash_table2<int>;

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


TEST_CASE("duplicate_set_search")
{
    using namespace leviathan::test;
    SET1 s1;
    SET2 s2;
    SET3 s3;
    UNORDERED_SET1 s4;
    UNORDERED_SET2 s5;
    UNORDERED_SET3 s6;
    UNORDERED_SET4 s7;

    random_insert(s1, s2, s3, s4, s5, s6, s7);
    
    BENCHMARK("std::set")
    {
        return search_test(s1);
    };

    BENCHMARK("sorted_list")
    {
        return search_test(s2);
    };

    BENCHMARK("skip_list")
    {
        return search_test(s3);
    };

    BENCHMARK("std::unordered_set")
    {
        return search_test(s4);
    };

    BENCHMARK("hash_table")
    {
        return search_test(s5);
    };

    BENCHMARK("hash_table2")
    {
        return search_test(s6);
    };

    BENCHMARK("hash_table___")
    {
        return search_test(s7);
    };

}

TEST_CASE("duplicate_set_remove")
{
    using namespace leviathan::test;
    SET1 s1;
    SET2 s2;
    SET3 s3;
    UNORDERED_SET1 s4;
    UNORDERED_SET2 s5;
    UNORDERED_SET4 s6;

    random_insert(s1, s2, s3, s4, s5, s6);
    
    BENCHMARK("std::set")
    {
        return remove_test(s1);
    };

    BENCHMARK("sorted_list")
    {
        return remove_test(s2);
    };

    BENCHMARK("skip_list")
    {
        return remove_test(s3);
    };

    BENCHMARK("std::unordered_set")
    {
        return remove_test(s4);
    };

    BENCHMARK("hash_table")
    {
        return remove_test(s5);
    };

    BENCHMARK("hash_table2")
    {
        return remove_test(s6);
    };

}



