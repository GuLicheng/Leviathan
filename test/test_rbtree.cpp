#include <lv_cpp/collections/internal/rbtree.hpp>

using leviathan::collections::_Rb_tree_node_base;
using leviathan::collections::rb_set;
using leviathan::collections::rb_map;
using SetT = rb_set<int>;
using MapT = rb_map<int, std::string>;
using TreeNodeT = _Rb_tree_node_base;


#include "test_tree.ixx"


TEST_CASE("rbtree color test")
{
    SetT ss;

    struct ColorChecker
    {
        void operator()(TreeNodeT* node, int cur_level)
        {
            if (node)
            {
                if (node->m_color == TreeNodeT::_S_red)
                    REQUIRE(node->m_parent->m_color == TreeNodeT::_S_black);
                this->operator()(node->m_left, cur_level + (node->m_color == TreeNodeT::_S_black));
                this->operator()(node->m_right, cur_level + (node->m_color == TreeNodeT::_S_black));
                return;
            }
            if (black_level == -1)
                black_level = cur_level;
            else
                REQUIRE(black_level == cur_level);
        }

        int black_level = -1;
    };

    std::random_device rd;
    for (int i = 0; i < 1024; ++i)  
    {
        int x = rd() % 10240;
        ss.insert(x);
    }

    REQUIRE(ss.root()->base()->m_color == TreeNodeT::_S_black);
    ColorChecker()(ss.root()->base(), 0);

    for (int i = 0; i < 1024; ++i)  
    {
        int x = rd() % 10240;
        ss.erase(x);
    }
    REQUIRE(ss.root()->base()->m_color == TreeNodeT::_S_black);
    ColorChecker()(ss.root()->base(), 0);



}
