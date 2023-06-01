#pragma once

#include <string>
#include <vector>
#include <queue>

#include "debug_base.hpp"
#include "avl_tree.hpp"

/*
    level 1:              2(rank = 1)
                        /           \
    level 2:         0(rank=2)       3(rank=3)
                         \
    level 3:               1(rank=5)

    assume Width = 3, Limit = 1, Offset = 0, level = 3, number_of_point = 6 


Level:    1 |   |   |   |   |   |   | * | * | * |   |   |   |   |   |   |
          2 |   |   | * | * | * |   |   |   |   |   | * | * | * |   |   |
          3 | * | * | * |   | * | * | * |   | * | * | * |   | * | * | * |
Index:        0   1   2   3   4   5   6   7   8   9   10  11  12  13  14

the maximum line is the last line and the length is: 
    2 ^ level * Width + (2 ^ level - 1) * Limit = 15

we can easily find that the position center of first point in each line is: 
    FirstCenterPoint(level) = length / (2 ^ level) 

and the number of blank in start of each line is:
    length / (2 ^ level) - [Width / 2]

and the number of blank between each point is:

*/


namespace leviathan::debug
{

    template <typename Node = void, int Width = 3, int Limit = 1, int Offset = 0>
    struct drawer
    {

        using node = Node;

        constexpr drawer(int level, int width = Width)
        {
            assert(level > 0);
            m_width = width;
            m_level = level;
            const auto cnt = 1 << (level - 1);
            m_length = cnt * m_width + (cnt - 1) * Limit + Offset; 
        }

        constexpr int start_point(int level)
        {
            return m_length >> level;
        }

        constexpr int start_offset(int level)
        {
            const auto center = start_point(level);
            return center - m_width / 2;
        }

        constexpr int limit(int level)
        {
            assert(level > 1 && level <= m_level);
            return delta(level) - (m_width / 2 * 2) - 1;
        }

        constexpr int delta(int level)
        {
            assert(level > 1 && level <= m_level);
            const auto this_level = start_point(level);
            const auto last_level = start_point(level - 1);
            return (last_level - this_level) * 2;
        }

        std::string repeat_character(int n, char c)
        {
            std::string s;
            s.append(n, c);
            return s;
        }

        std::string blank(int n)
        { return repeat_character(n, ' '); }

        int m_length;
        int m_level;
        int m_width;

    };

    template <typename T, typename Compare, typename Allocator, typename KeyOfValue, bool UniqueKey>
    struct tree_traits<::leviathan::collections::avl_tree<T, Compare, Allocator, KeyOfValue, UniqueKey>>
    {
        using tree_type = ::leviathan::collections::avl_tree<T, Compare, Allocator, KeyOfValue, UniqueKey>;
        using node_type = typename tree_type::tree_node;

        // Convert node to string
        static std::string to_string(node_type* x)
        {
            const auto& value = x->m_val;
            return std::to_string(value);
        }

        // Return left child of x
        static node_type* left(node_type* x)
        { return static_cast<node_type*>(x->m_left); }

        // Return right child of x
        static node_type* right(node_type* x)
        { return static_cast<node_type*>(x->m_right); }

        static node_type* root(tree_type* tree)
        { return tree->root(); }

    };

    template <typename T, typename Compare, typename Allocator>
    struct tree_traits<::leviathan::collections::avl_set<T, Compare, Allocator>> 
        : tree_traits<::leviathan::collections::avl_tree<T, Compare, Allocator, ::leviathan::collections::identity, true>> { };


    template <typename Node>
    struct TreeNode
    {
        Node* node; // pointer to raw node, nullptr if not exist
        int level;  // height of this node, the root' height is 1
        int rank;   // the rank in a full binary tree

        int level_rank() const 
        {
            auto cnt = (1 << (level - 1)) - 1;
            return rank - cnt - 1;
        }

        friend std::ostream& operator<<(std::ostream& os, const TreeNode& node)
        {
            os << "(" << node.level << " - " << node.rank << ")";
            return os;
        } 

    };

/*
        2
    ┏━━━┻━━━┓ 
    0       3
    ┗━┓
      1

    "┻": // node with left and right child
    "━": // just for fill
    "┗": // only with right child
    "┛": // only with left child
    "┏": // left child
    "┓":  // right child

*/
    template <typename Tree>
    struct level_order_printer
    {

        using tree_node = typename Tree::tree_node;
        using node_type = TreeNode<tree_node>;
        using traits = tree_traits<Tree>;

        std::vector<std::vector<node_type>> m_nodes;
        Tree* m_tree;

        level_order_printer(Tree& t) : m_tree{ std::addressof(t) } { }

    private:
        void traverse()
        {
            m_nodes.clear();

            std::queue<tree_node*> q1;
            std::queue<node_type> q2;

            int level = 1;
            auto root = traits::root(m_tree);
            q1.emplace(root); // 
            q2.emplace(root, level, 1);

            while (q1.size())
            {
                level++;
                std::vector<tree_node*> qq1;
                std::vector<node_type> qq2;
                m_nodes.emplace_back(); //  add new line

                while (q1.size())
                {
                    auto n1 = q1.front(); q1.pop();
                    auto n2 = q2.front(); q2.pop();
                    m_nodes.back().emplace_back(n2);

                    auto l = traits::left(n1), r = traits::right(n1);
                    if (l) 
                    {
                        qq1.emplace_back(l);
                        qq2.emplace_back(l, level, n2.rank * 2);
                    }
                    if (r) 
                    {
                        qq1.emplace_back(r);
                        qq2.emplace_back(r, level, n2.rank * 2 + 1);
                    }
                }

                for (auto i : qq1) q1.emplace(i);
                for (auto i : qq2) q2.emplace(i);

            }
            
        }
        
    public:
        void show()
        {
            traverse();

            int level = m_nodes.size();

            if (level == 0)
            {
                std::cout << "Empty Tree\n";
                return;
            }

            drawer<tree_node> d(level);

            for (int level = 1; level <= (int)m_nodes.size(); ++level)
            {
                auto& nodes = m_nodes[level - 1];
                for (int j = 0, k = 0; j < 1 << (level - 1); ++j)
                {
                    auto node = nodes[k];
                    if (node.level_rank() == j)
                    {
                        // this position is node
                        std::string info = traits::to_string(node.node).substr(0, 3);
                        if (info.size() < 3)
                        {
                            // make center
                            info.append(3 - info.size(), ' ');
                        }
                        if (j == 0)
                        {
                            std::cout << d.blank(d.start_offset(level))
                                << info;
                        }
                        else
                        {
                            std::cout << d.blank(d.limit(level))
                                << info;
                        }
                        k++;
                    }
                    else
                    {
                        // this position is nullptr
                        if (j == 0)
                        {
                            std::cout << d.blank(d.start_offset(level))
                                << d.repeat_character(3, '*');
                        }
                        else
                        {
                            std::cout << d.blank(d.limit(level))
                                << d.repeat_character(3, '*');
                        }
                    }
                }
                std::cout << '\n';
            }


        }

    };

    template <typename Tree>
    struct in_order_printer
    {
        using tree_type = Tree;
        using node_type = typename Tree::tree_node;
        using traits = tree_traits<Tree>;

        Tree* m_tree;

        in_order_printer(Tree& tree) : m_tree{ std::addressof(tree) } { }

    private:
        // https://github.com/emirpasic/gods/blob/dbdbadc158ae6b453820b3cfb8c6cb48be4d7ddf/trees/avltree/avltree.go#L332
        void output(node_type* node, std::string prefix, bool isTail, std::string& str)
        {
            if (traits::right(node))
            {
                auto newPrefix = prefix;
                if (isTail)
                    newPrefix += "│   ";
                else    
                    newPrefix += "    ";
                output(traits::right(node), newPrefix, false, str);
            }

            str += prefix;
            if (isTail)
                str += "└── ";
            else
                str += "┌── ";

            str += traits::to_string(node) + "\n";

            if (traits::left(node))
            {
                auto newPrefix = prefix;
                if (isTail)
                    newPrefix += "    ";
                else    
                    newPrefix += "│   ";
                output(traits::left(node), newPrefix, true, str);
            }
        }

    public:
        void show()
        {
            auto root = traits::root(m_tree);
            if (!root)
            {
                std::cout << "Empty Tree\n";
                return;
            }
            std::string context;
            output(root, "", true, context);
            std::cout << "Tree: \n" << context << std::endl;
        }

    };


} // end of namespace leviathan::debug