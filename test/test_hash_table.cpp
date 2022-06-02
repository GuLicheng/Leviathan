#define CATCH_CONFIG_MAIN

#include "test_container.hpp"

#include <lv_cpp/collections/py_dict.hpp>

USING_TEST

TEST_CASE("duplicate_hashtable")
{
    using SET = leviathan::collections::hash_table<int>;
    simple_unique_set_container_test<SET>();
    unique_set_insert_method<SET>();
    simple_unique_set_container_random_test<SET>(false);
}













