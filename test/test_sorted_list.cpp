#define CATCH_CONFIG_MAIN

#include "test_container.hpp"

#include <lv_cpp/collections/sorted_list.hpp>

#include <set>

USING_TEST

TEST_CASE("DEBUG")
{
    using STL_SET = std::set<int>;
    simple_unique_set_container_test<STL_SET>();
    unique_set_search_method<STL_SET>();
    simple_unique_set_iterator_test<STL_SET>();
    unique_set_insert_method<STL_SET>();

    using STL_MAP = std::map<std::string, int>;
    simple_unique_map_container_test<STL_MAP>();
}


TEST_CASE("sorted_list_duplicate")
{
    using namespace leviathan::collections;
    using SET = sorted_list<int>;
    simple_unique_set_container_test<SET>();
    unique_set_search_method<SET>();
    simple_unique_set_iterator_test<SET>();
    simple_unique_set_container_random_test<SET>();
    unique_set_insert_method<SET>();
}

TEST_CASE("sorted_map_duplicate")
{
    using namespace leviathan::collections;
    using MAP = sorted_map<std::string, int>;

    simple_unique_map_container_test<MAP>();
}

TEST_CASE("constructor")
{

    std::vector default_values = { 1, 2, 3, 4, 5 };

    // copy constructor
    using namespace leviathan::collections;

    using set_type = sorted_list<int>;

    set_type sl;

    for (auto val : default_values) sl.insert(val);
    

}
