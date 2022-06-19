#define CATCH_CONFIG_MAIN


#include "test_time.hpp"

#include <set>
#include <unordered_set>

#include <lv_cpp/collections/internal/raw_hash_table.hpp>
#include <lv_cpp/collections/hash_table.hpp>

using UNORDERED_SET1 = std::unordered_set<int>;
using UNORDERED_SET2 = ::leviathan::collections2::hash_table<int>;
using UNORDERED_SET3 = ::leviathan::collections::hash_set<int>;

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

    REQUIRE(s1.size() == s2.size());
    REQUIRE(s1.size() == s3.size());

}



