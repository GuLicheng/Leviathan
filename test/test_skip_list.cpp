#define CATCH_CONFIG_MAIN

#include "test_container.hpp"

#include <lv_cpp/collections/skip_list.hpp>

USING_TEST

TEST_CASE("duplicate_skip_list")
{
    using namespace leviathan::collections;

    using SET = skip_list<int>;

    simple_unique_set_container_test<SET>();
    unique_set_search_method<SET>();
    simple_unique_set_iterator_test<SET>();
    simple_unique_set_container_random_test<SET>();
    unique_set_insert_method<SET>();
}

TEST_CASE("duplicate_skip_list_map")
{
    using namespace leviathan::collections;
    using MAP = skip_map<std::string, int>;
    simple_unique_map_container_test<MAP>();
}











