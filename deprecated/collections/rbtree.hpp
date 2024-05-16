/*
    The code is borrow from: 
    https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B98/tree.cc
*/

#pragma once

#include "tree_node.hpp"

namespace leviathan::collections
{
    struct _Rb_tree_node_base : tree_node_basic_operation<_Rb_tree_node_base>
    {
        // Different form stl_tree, we add another color sentinel, for each node except header, the
        // color is red or black. And in this way, the iterator can be a circle.
        // Since compiler will fill some bytes for struct, it will always occupy sizeof(_Rb_tree_node_base*) bytes. 
        enum color
        {
            _S_red,
            _S_black,
            _S_sentinel
        };


        static constexpr void reset(_Rb_tree_node_base* node)
        {
            node->m_parent = nullptr;
            node->m_left = node->m_right = node;
            node->m_color = _S_sentinel;
        }

        static constexpr void init(_Rb_tree_node_base* node)
        {
            node->m_left = node->m_right = node->m_parent = nullptr;
            node->m_color = _S_red;
        }

        static constexpr bool is_header(_Rb_tree_node_base* node)
        { return node->m_color == _S_sentinel; } 

        static constexpr void clone(_Rb_tree_node_base* x, const _Rb_tree_node_base* y)
        { x->m_color = y->m_color; } 

        static constexpr void
        insert_and_rebalance(const bool __insert_left,
                                      _Rb_tree_node_base *__x,
                                      _Rb_tree_node_base *__p,
                                      _Rb_tree_node_base &__header) 
        {
            __header.m_color = _S_red;
            _Rb_tree_node_base *&__root = __header.m_parent;

            // Initialize fields in new node to insert.
            __x->m_parent = __p;
            __x->m_left = 0;
            __x->m_right = 0;
            __x->m_color = _S_red;

            // Insert.
            // Make new node child of parent and maintain root, leftmost and
            // rightmost nodes.
            // N.B. First node is always inserted left.
            if (__insert_left)
            {
                __p->m_left = __x; // also makes leftmost = __x when __p == &__header

                if (__p == &__header)
                {
                    __header.m_parent = __x;
                    __header.m_right = __x;
                }
                else if (__p == __header.m_left)
                    __header.m_left = __x; // maintain leftmost pointing to min node
            }
            else
            {
                __p->m_right = __x;

                if (__p == __header.m_right)
                    __header.m_right = __x; // maintain rightmost pointing to max node
            }
            // Rebalance.
            while (__x != __root && __x->m_parent->m_color == _S_red)
            {
                _Rb_tree_node_base *const __xpp = __x->m_parent->m_parent;

                if (__x->m_parent == __xpp->m_left)
                {
                    _Rb_tree_node_base *const __y = __xpp->m_right;
                    if (__y && __y->m_color == _S_red)
                    {
                        __x->m_parent->m_color = _S_black;
                        __y->m_color = _S_black;
                        __xpp->m_color = _S_red;
                        __x = __xpp;
                    }
                    else
                    {
                        if (__x == __x->m_parent->m_right)
                        {
                            __x = __x->m_parent;
                            tree_rotate_left(__x, __root);
                        }
                        __x->m_parent->m_color = _S_black;
                        __xpp->m_color = _S_red;
                        tree_rotate_right(__xpp, __root);
                    }
                }
                else
                {
                    _Rb_tree_node_base *const __y = __xpp->m_left;
                    if (__y && __y->m_color == _S_red)
                    {
                        __x->m_parent->m_color = _S_black;
                        __y->m_color = _S_black;
                        __xpp->m_color = _S_red;
                        __x = __xpp;
                    }
                    else
                    {
                        if (__x == __x->m_parent->m_left)
                        {
                            __x = __x->m_parent;
                            tree_rotate_right(__x, __root);
                        }
                        __x->m_parent->m_color = _S_black;
                        __xpp->m_color = _S_red;
                        tree_rotate_left(__xpp, __root);
                    }
                }
            }
            __root->m_color = _S_black;
            __header.m_color = _S_sentinel;
        }

        static constexpr _Rb_tree_node_base *
        rebalance_for_erase(_Rb_tree_node_base *const __z,
                                     _Rb_tree_node_base &__header) 
        {

            __header.m_color = _S_red;

            _Rb_tree_node_base *&__root = __header.m_parent;
            _Rb_tree_node_base *&__leftmost = __header.m_left;
            _Rb_tree_node_base *&__rightmost = __header.m_right;
            _Rb_tree_node_base *__y = __z;
            _Rb_tree_node_base *__x = 0;
            _Rb_tree_node_base *__x_parent = 0;

            if (__y->m_left == 0)       // __z has at most one non-null child. y == z.
                __x = __y->m_right;     // __x might be null.
            else if (__y->m_right == 0) // __z has exactly one non-null child. y == z.
                __x = __y->m_left;      // __x is not null.
            else
            {
                // __z has two non-null children.  Set __y to
                __y = __y->m_right; //   __z's successor.  __x might be null.
                while (__y->m_left != 0)
                    __y = __y->m_left;
                __x = __y->m_right;
            }
            if (__y != __z)
            {
                // relink y in place of z.  y is z's successor
                __z->m_left->m_parent = __y;
                __y->m_left = __z->m_left;
                if (__y != __z->m_right)
                {
                    __x_parent = __y->m_parent;
                    if (__x)
                        __x->m_parent = __y->m_parent;
                    __y->m_parent->m_left = __x; // __y must be a child of m_left
                    __y->m_right = __z->m_right;
                    __z->m_right->m_parent = __y;
                }
                else
                    __x_parent = __y;
                if (__root == __z)
                    __root = __y;
                else if (__z->m_parent->m_left == __z)
                    __z->m_parent->m_left = __y;
                else
                    __z->m_parent->m_right = __y;
                __y->m_parent = __z->m_parent;
                std::swap(__y->m_color, __z->m_color);
                __y = __z;
                // __y now points to node to be actually deleted
            }
            else
            { // __y == __z
                __x_parent = __y->m_parent;
                if (__x)
                    __x->m_parent = __y->m_parent;
                if (__root == __z)
                    __root = __x;
                else if (__z->m_parent->m_left == __z)
                    __z->m_parent->m_left = __x;
                else
                    __z->m_parent->m_right = __x;
                if (__leftmost == __z)
                {
                    if (__z->m_right == 0) // __z->m_left must be null also
                        __leftmost = __z->m_parent;
                    // makes __leftmost == _M_header if __z == __root
                    else
                        __leftmost = _Rb_tree_node_base::minimum(__x);
                }
                if (__rightmost == __z)
                {
                    if (__z->m_left == 0) // __z->m_right must be null also
                        __rightmost = __z->m_parent;
                    // makes __rightmost == _M_header if __z == __root
                    else // __x == __z->m_left
                        __rightmost = _Rb_tree_node_base::maximum(__x);
                }
            }
            if (__y->m_color != _S_red)
            {
                while (__x != __root && (__x == 0 || __x->m_color == _S_black))
                    if (__x == __x_parent->m_left)
                    {
                        _Rb_tree_node_base *__w = __x_parent->m_right;
                        if (__w->m_color == _S_red)
                        {
                            __w->m_color = _S_black;
                            __x_parent->m_color = _S_red;
                            tree_rotate_left(__x_parent, __root);
                            __w = __x_parent->m_right;
                        }
                        if ((__w->m_left == 0 ||
                             __w->m_left->m_color == _S_black) &&
                            (__w->m_right == 0 ||
                             __w->m_right->m_color == _S_black))
                        {
                            __w->m_color = _S_red;
                            __x = __x_parent;
                            __x_parent = __x_parent->m_parent;
                        }
                        else
                        {
                            if (__w->m_right == 0 || __w->m_right->m_color == _S_black)
                            {
                                __w->m_left->m_color = _S_black;
                                __w->m_color = _S_red;
                                tree_rotate_right(__w, __root);
                                __w = __x_parent->m_right;
                            }
                            __w->m_color = __x_parent->m_color;
                            __x_parent->m_color = _S_black;
                            if (__w->m_right)
                                __w->m_right->m_color = _S_black;
                            tree_rotate_left(__x_parent, __root);
                            break;
                        }
                    }
                    else
                    {
                        // same as above, with m_right <-> m_left.
                        _Rb_tree_node_base *__w = __x_parent->m_left;
                        if (__w->m_color == _S_red)
                        {
                            __w->m_color = _S_black;
                            __x_parent->m_color = _S_red;
                            tree_rotate_right(__x_parent, __root);
                            __w = __x_parent->m_left;
                        }
                        if ((__w->m_right == 0 ||
                             __w->m_right->m_color == _S_black) &&
                            (__w->m_left == 0 ||
                             __w->m_left->m_color == _S_black))
                        {
                            __w->m_color = _S_red;
                            __x = __x_parent;
                            __x_parent = __x_parent->m_parent;
                        }
                        else
                        {
                            if (__w->m_left == 0 || __w->m_left->m_color == _S_black)
                            {
                                __w->m_right->m_color = _S_black;
                                __w->m_color = _S_red;
                                //   tree_rotate_left(__w, __root);
                                tree_rotate_left(__w, __root);
                                __w = __x_parent->m_left;
                            }
                            __w->m_color = __x_parent->m_color;
                            __x_parent->m_color = _S_black;
                            if (__w->m_left)
                                __w->m_left->m_color = _S_black;
                            //   tree_rotate_right(__x_parent, __root);
                            tree_rotate_right(__x_parent, __root);
                            break;
                        }
                    }
                if (__x)
                    __x->m_color = _S_black;
            }
            
            __header.m_color = _S_sentinel;            
            return __y;
        }


        color m_color;

    };

    static_assert(tree_node_interface<_Rb_tree_node_base>);

}

#include "tree.hpp"


namespace leviathan::collections
{
    template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
    using rb_set = tree_set<T, Compare, Allocator, _Rb_tree_node_base>;

    template <typename T, typename Compare = std::less<>>
    using pmr_rb_set = tree_set<T, Compare, std::pmr::polymorphic_allocator<T>, _Rb_tree_node_base>;  

    template <typename K, typename V, typename Compare = std::less<>, typename Allocator = std::allocator<std::pair<const K, V>>>
    using rb_map = tree_map<K, V, Compare, Allocator, _Rb_tree_node_base>;

    template <typename K, typename V, typename Compare = std::less<>>
    using pmr_rb_map = tree_map<K, V, Compare, std::pmr::polymorphic_allocator<std::pair<const K, V>>, _Rb_tree_node_base>;

}
