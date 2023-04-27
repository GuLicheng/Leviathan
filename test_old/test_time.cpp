#define CATCH_CONFIG_MAIN


#include "test_time.hpp"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>

// #include <lv_cpp/collections/sorted_list.hpp>
#include <lv_cpp/collections/internal/skip_list.hpp>
#include <lv_cpp/collections/internal/py_hash.hpp>
#include <lv_cpp/collections/internal/avl_tree.hpp>
#include <lv_cpp/collections/internal/rbtree.hpp>
#include <lv_cpp/collections/internal/binary_search_tree.hpp>
// #include <lv_cpp/collections/internal/raw_hash_table.hpp>


using SET1 = std::set<int>;
using SET2 = leviathan::collections::skip_set<int>;
using SET3 = leviathan::collections::avl_set<int>;
using SET4 = leviathan::collections::rb_set<int>;
using SET5 = leviathan::collections::binary_set<int>;

using UNORDERED_SET1 = std::unordered_set<int>;
using UNORDERED_SET2 = ::leviathan::collections::hash_set<int>;

TEST_CASE("duplicate_ordered_collections_random_insert")
{
    using namespace leviathan::test;

    BENCHMARK("std::set")
    {
        return random_insert_test<SET1>();
    };

    BENCHMARK("skip_list")
    {
        return random_insert_test<SET2>();
    };

    BENCHMARK("avl_set")
    {
        return random_insert_test<SET3>();
    };

    BENCHMARK("rb_set")
    {
        return random_insert_test<SET4>();
    };

    BENCHMARK("binary_set")
    {
        return random_insert_test<SET5>();
    };

}

TEST_CASE("duplicate_ordered_collections_ascending_insert")
{
    using namespace leviathan::test;

    BENCHMARK("std::set")
    {
        return ascending_insert_test<SET1>();
    };

    BENCHMARK("skip_list")
    {
        return ascending_insert_test<SET2>();
    };

    BENCHMARK("avl_set")
    {
        return ascending_insert_test<SET3>();
    };

    BENCHMARK("rb_set")
    {
        return ascending_insert_test<SET4>();
    };
}

TEST_CASE("duplicate_ordered_collections_descending_insert")
{
    using namespace leviathan::test;

    BENCHMARK("std::set")
    {
        return descending_insert_test<SET1>();
    };

    BENCHMARK("skip_list")
    {
        return descending_insert_test<SET2>();
    };

    BENCHMARK("avl_set")
    {
        return descending_insert_test<SET3>();
    };

    BENCHMARK("rb_set")
    {
        return descending_insert_test<SET4>();
    };
}

TEST_CASE("duplicate_unordered_collections_random_insert")
{
    using namespace leviathan::test;
    UNORDERED_SET1 s1;
    UNORDERED_SET2 s2;

    BENCHMARK("std::unordered_set")
    {
        return random_insert_test<UNORDERED_SET1>();
    };

    BENCHMARK("hash_table")
    {
        return random_insert_test<UNORDERED_SET2>();
    };
}

TEST_CASE("duplicate_unordered_search")
{
    using namespace leviathan::test;
    UNORDERED_SET1 s1;
    UNORDERED_SET2 s2;

    random_insert(s1, s2);
    
    BENCHMARK("std::unordered_set")
    {
        return search_test(s1);
    };

    BENCHMARK("hash_table")
    {
        return search_test(s2);
    };

}

TEST_CASE("duplicate_unordered_remove")
{
    using namespace leviathan::test;
    UNORDERED_SET1 s1;
    UNORDERED_SET2 s2;

    random_insert(s1, s2);
    
    BENCHMARK("std::unordered_set")
    {
        return remove_test(s1);
    };

    BENCHMARK("hash_table")
    {
        return remove_test(s2);
    };

}



