#define CATCH_CONFIG_MAIN

#include "test_container.hpp"

#include <lv_cpp/collections/hash_table.hpp>
// #include <lv_cpp/collections/hash_table2.hpp>

USING_TEST

TEST_CASE("duplicate_hashtable")
{
    using SET = leviathan::collections::hash_table<int>;
    simple_unique_set_container_test<SET>();
    unique_set_insert_method<SET>();
    simple_unique_set_container_random_test<SET>(false);
    simple_unique_set_iterator_test<SET>(false);
}

TEST_CASE("quadric_hash")
{
    using SET = leviathan::collections::hash_table_impl<
        int, 
        std::hash<int>, 
        std::equal_to<>, 
        std::allocator<int>, 
        leviathan::collections::hash_set_config<int, std::hash<int>, std::equal_to<>, std::allocator<int>>, 
        leviathan::collections::detail::quadratic_policy<>, 
        true, 
        true, 
        17>;
    simple_unique_set_container_test<SET>();
    unique_set_insert_method<SET>();
    simple_unique_set_container_random_test<SET>(false);
    simple_unique_set_iterator_test<SET>(false);
}

TEST_CASE("duplicate_hashtable_map")
{
    using namespace leviathan::collections;
    using MAP = leviathan::collections::hash_map<std::string, int>;

    simple_unique_map_container_test<MAP>();
}


TEST_CASE("policy")
{
    using policy1 = ::leviathan::collections::detail::python_dict<>;
    
    // 0 -> 1 -> 6 -> 7 -> 4 -> 5 -> 2 -> 3 -> 0 [and here it's repeating]
    
    SECTION("python_dict")
    {
        auto p = policy1(0, 8);

        REQUIRE(p.first() == 0);
        REQUIRE(p() == 1);
        REQUIRE(p() == 6);
        REQUIRE(p() == 7);
        REQUIRE(p() == 4);
        REQUIRE(p() == 5);
        REQUIRE(p() == 2);
        REQUIRE(p() == 3);
        REQUIRE(p() == 0);
    }

}


TEST_CASE("hash_table2")
{
    using SET = leviathan::collections::hash_table2<int>;
    simple_unique_set_container_random_test<SET>(false);
}





