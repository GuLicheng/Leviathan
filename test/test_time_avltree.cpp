#define CATCH_CONFIG_MAIN


#include "test_time.hpp"

#include <set>
#include <lv_cpp/collections/internal/avl_tree.hpp>


using Tree1 = std::set<int>;
using Tree2 = leviathan::collections::avl_set<int>;

TEST_CASE("duplicate_ordered_insert")
{
    using namespace leviathan::test;

    BENCHMARK("std::set")
    {
        Tree1 s1;
        return random_insert(s1);
    };

    BENCHMARK("avl_tree")
    {
        Tree2 s2;
        return random_insert(s2);
    };
}

TEST_CASE("duplicate_ordered_search")
{
    using namespace leviathan::test;
    Tree1 s1;
    Tree2 s2;

    random_insert(s1, s2);

    BENCHMARK("std::set")
    {
        return search_test(s1);
    };

    BENCHMARK("avl_tree")
    {
        return search_test(s2);
    };
}

#if 0
TEST_CASE("duplicate_ordered_remove")
{
    using namespace leviathan::test;
    Tree1 s1;
    Tree2 s2;

    random_insert(s1, s2);
    
    REQUIRE(s1.size() == s2.size());
    REQUIRE(std::ranges::equal(s1, s2));

    BENCHMARK("std::set")
    {
        return remove_test(s1);
    };

    BENCHMARK("avl_tree")
    {
        return remove_test(s2);
    };

    REQUIRE(std::ranges::equal(s1, s2));
    REQUIRE(std::ranges::distance(s1) == std::ranges::distance(s2));
    REQUIRE(s1.size() == s2.size());
}
#endif


