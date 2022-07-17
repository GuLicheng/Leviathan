#define CATCH_CONFIG_MAIN

#include <lv_cpp/collections/internal/avl_tree.hpp>
#include <thirdpart/catch.hpp>

using leviathan::collections::avl_node_base;
using leviathan::collections::avl_set;
using SetT = avl_set<int>;
 
void test_height_and_value()
{
    using T = avl_set<int>;

    T avl;
    avl.insert(5 - 1);
    avl.insert(6 - 1);
    avl.insert(7 - 1);
    avl.insert(3 - 1);
    avl.insert(4 - 1);
    avl.insert(1 - 1);
    avl.insert(2 - 1);

    /*
                    3
                1       5
              0   2   4   6
    */

    using node_type = T::tree_node;

    auto root_value = [](const avl_node_base* p) {
        return static_cast<const node_type*>(p)->m_val;
    };

    auto root = avl.root();

    REQUIRE(root->m_height == 3);

    REQUIRE(root_value(root) == 3);
    REQUIRE(root_value(root->m_left) == 1);
    REQUIRE(root_value(root->m_right) == 5);
    REQUIRE(root_value(root->m_left->m_left) == 0);
    REQUIRE(root_value(root->m_left->m_right) == 2);
    REQUIRE(root_value(root->m_right->m_right) == 6);
    REQUIRE(root_value(root->m_right->m_left) == 4);


    struct HeightChecker
    {
        void operator()(avl_node_base* p)
        {
            if (p)
            {
                this->operator()(p->m_left);
                this->operator()(p->m_right);

                int lh = avl_node_base::height(p->m_left);
                int rh = avl_node_base::height(p->m_right);
                int diff = lh - rh;
                REQUIRE(-1 <= diff);
                REQUIRE(diff <= 1);
                REQUIRE(p->m_height == std::max(lh, rh) + 1);
                // check leaf height
                if (!p->m_left && !p->m_right)
                    REQUIRE(p->m_height == 1);
            }
        }
    };

    HeightChecker()(static_cast<avl_node_base*>(root));

    T empty_tree;
    REQUIRE(empty_tree.root() == nullptr);

    T single_element_tree;
    single_element_tree.insert(1);
    REQUIRE(single_element_tree.root()->m_height == 1);
}

TEST_CASE("avl_tree_height_test")
{
    test_height_and_value();
}

TEST_CASE("insert elements", "[iterator][insert][emplace][emplace_hint]")
{

    SetT h;
    
    SECTION("insert")
    {
        REQUIRE(h.insert(1).second == true);
        REQUIRE(*(h.insert(1).first) == 1);
        REQUIRE(h.insert(1).second == false);
    }

    SECTION("emplace")
    {
        REQUIRE(h.emplace(1).second == true);
        REQUIRE(*(h.emplace(1).first) == 1);
        REQUIRE(h.emplace(1).second == false);
    }

    SECTION("insert with iterator")
    {
        auto values = { 1, 2, 3, 4, 5 };
        std::ranges::copy(values, std::inserter(h, h.end()));
        REQUIRE(std::ranges::distance(h) == 5);
    }

    SECTION("emplace with iterator")
    {
        auto values = { 1, 2, 3, 4, 5 };
        std::ranges::for_each(values, [&](int x) {
            h.emplace_hint(h.end(), x);
        });
        REQUIRE(std::ranges::distance(h) == 5);
    }

}

TEST_CASE("observer", "[empty][size]")
{
    SetT h;
    REQUIRE(h.size() == 0);
    REQUIRE(h.empty());
}

TEST_CASE("search elements", "[iterator][contains][find][lower_bound][upper_bound][equal_range][count]")
{
    SetT h;

    h.insert(1);
    h.insert(3);
    h.insert(5);
    h.insert(7);
    h.insert(9);

    // contains
    REQUIRE(h.contains(1));
    REQUIRE(!h.contains(0));

    // find
    REQUIRE(h.find(1) != h.end());
    REQUIRE(h.find(2) == h.end());
    REQUIRE(h.find(3) != h.end());

    // lower_bound
    REQUIRE(*h.lower_bound(1) == 1);
    REQUIRE(*h.lower_bound(2) == 3);
    REQUIRE(h.lower_bound(10) == h.end());

    // upper_bound
    REQUIRE(*h.upper_bound(1) == 3);
    REQUIRE(*h.upper_bound(2) == 3);
    REQUIRE(*h.upper_bound(3) == 5);
    REQUIRE(h.upper_bound(9) == h.end());
    REQUIRE(h.upper_bound(10) == h.end());

    // equal_range
    auto [a1, b1] = h.equal_range(0);
    REQUIRE(a1 == b1);
    auto [a2, b2] = h.equal_range(1);
    REQUIRE(a2 != b2);
    auto [a3, b3] = h.equal_range(10);
    REQUIRE(a3 == b3);

    // count
    REQUIRE(h.count(0) == 0);
    REQUIRE(h.count(1) == 1);

}

