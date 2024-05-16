/*
    The code is borrow from: 
    https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B98/tree.cc
    We just want to test our tree interface is correct for red black tree.
*/
#pragma once

#include "tree_node_operation.hpp"

namespace leviathan::collections
{
    
struct _Rb_tree_node_base : binary_node_operation
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

    _Rb_tree_node_base* m_parent;
    _Rb_tree_node_base* m_left;
    _Rb_tree_node_base* m_right;

    _Rb_tree_node_base* lchild() { return m_left; }
    const _Rb_tree_node_base* lchild() const { return m_left; }
    void lchild(_Rb_tree_node_base* node) { m_left = node; }

    _Rb_tree_node_base* rchild() { return m_right; }
    const _Rb_tree_node_base* rchild() const { return m_right; }
    void rchild(_Rb_tree_node_base* node) { m_right = node; }

    _Rb_tree_node_base* parent() { return m_parent; }
    const _Rb_tree_node_base* parent() const { return m_parent; }
    void parent(_Rb_tree_node_base* node) { m_parent = node; }


    static constexpr void reset(_Rb_tree_node_base* node)
    {
        node->m_parent = nullptr;
        node->m_left = node->m_right = node;
        node->m_color = _S_sentinel;
    }

    void as_empty_tree_header()
    {
        _Rb_tree_node_base::reset(this);
    }

    static constexpr void init(_Rb_tree_node_base* node)
    {
        node->m_left = node->m_right = node->m_parent = nullptr;
        node->m_color = _S_red;
    }

    void init()
    {
        _Rb_tree_node_base::init(this);
    }

    static constexpr bool is_header(const _Rb_tree_node_base *node)
    {
        return node->m_color == _S_sentinel;
    }

    bool is_header() const
    {
        return _Rb_tree_node_base::is_header(this);
    }

    static constexpr void clone(_Rb_tree_node_base *x, const _Rb_tree_node_base *y)
    {
        x->m_color = y->m_color;
    }

    void clone(const _Rb_tree_node_base* y)
    {
        _Rb_tree_node_base::clone(this, y);
    }

    void insert_and_rebalance(bool insert_left, _Rb_tree_node_base* p, _Rb_tree_node_base& header)
    {
        _Rb_tree_node_base::insert_and_rebalance(
            insert_left,
            this, 
            p, 
            header
        );
    }

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

    _Rb_tree_node_base* rebalance_for_erase(_Rb_tree_node_base& header)
    {
        return _Rb_tree_node_base::rebalance_for_erase(this, header);
    }

    static constexpr _Rb_tree_node_base* maximum(_Rb_tree_node_base* x)
    {
        assert(x && "x should not be nullptr");
        for (; x->m_right; x = x->m_right);
        return x;
    }

    _Rb_tree_node_base* minimum() { return _Rb_tree_node_base::minimum(this); }
    const _Rb_tree_node_base* minimum() const { return _Rb_tree_node_base::minimum(this); }

    _Rb_tree_node_base* maximum() { return _Rb_tree_node_base::maximum(this); }
    const _Rb_tree_node_base* maximum() const { return _Rb_tree_node_base::maximum(this); }

    static constexpr _Rb_tree_node_base* minimum(_Rb_tree_node_base* x)
    {
        assert(x && "x should not be nullptr");
        for (; x->m_left; x = x->m_left);
        return x;
    }

    static constexpr const _Rb_tree_node_base* maximum(const _Rb_tree_node_base* x)
    { return maximum(const_cast<_Rb_tree_node_base*>(x)); }

    static constexpr const _Rb_tree_node_base* minimum(const _Rb_tree_node_base* x)
    { return minimum(const_cast<_Rb_tree_node_base*>(x)); }

    /*
    *     x            y              
    *       \   =>   /    
    *         y    x
    */
    static constexpr void tree_rotate_left(_Rb_tree_node_base* x, _Rb_tree_node_base*& root)
    {
        _Rb_tree_node_base* y = x->m_right;

        x->m_right = y->m_left;
        if (y->m_left != 0)
            y->m_left->m_parent = x;
        y->m_parent = x->m_parent;

        // x->parent will never be nullptr, since header->parent == root and root->parent == header
        if (x == root)
            root = y;
        else if (x == x->m_parent->m_left) 
            x->m_parent->m_left = y;
        else
            x->m_parent->m_right = y;
        y->m_left = x;
        x->m_parent = y;
    }


    /*
    *     x        y                   
    *    /     =>    \
    *   y              x
    */
    static constexpr void tree_rotate_right(_Rb_tree_node_base* x, _Rb_tree_node_base*& root)
    {
        _Rb_tree_node_base* y = x->m_left;

        x->m_left = y->m_right;
        if (y->m_right != 0)
            y->m_right->m_parent = x;
        y->m_parent = x->m_parent;

        if (x == root)
            root = y;
        else if (x == x->m_parent->m_right)
            x->m_parent->m_right = y;
        else
            x->m_parent->m_left = y;
        y->m_right = x;
        x->m_parent = y;
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


using red_black_node = _Rb_tree_node_base;

} // namespace leviathan


