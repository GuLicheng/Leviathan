/*
    The code is borrowed from: 
    https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B98/tree.cc
    We just want to test our tree interface is correct for red black tree.
*/
#pragma once

#include "tree_node_operation.hpp"

namespace leviathan::collections
{
    
struct _Rb_tree_node_base : binary_node_operation
{
    // Different from standard red black tree, we add another color sentinel, 
    // for each node except header, the color is red or black. And in this way, 
    // the iterator can be a circle. Since compiler will fill some bytes for 
    // struct, it will always occupy sizeof(_Rb_tree_node_base*) bytes. 
    enum _Rb_tree_color
    {
        _S_red,
        _S_black,
        _S_sentinel
    };

    _Rb_tree_node_base* _M_parent;
    _Rb_tree_node_base* _M_left;
    _Rb_tree_node_base* _M_right;
    _Rb_tree_color _M_color;

    auto& lchild() { return _M_left; }
    auto& lchild() const { return _M_left; }

    auto& rchild() { return _M_right; }
    auto& rchild() const { return _M_right; }

    auto& parent() { return _M_parent; }
    auto& parent() const { return _M_parent; }

    // _Rb_tree_node_base* lchild() { return _M_left; }
    // const _Rb_tree_node_base* lchild() const { return _M_left; }
    void lchild(_Rb_tree_node_base* node) { _M_left = node; }

    // _Rb_tree_node_base* rchild() { return _M_right; }
    // const _Rb_tree_node_base* rchild() const { return _M_right; }
    void rchild(_Rb_tree_node_base* node) { _M_right = node; }

    // _Rb_tree_node_base* parent() { return _M_parent; }
    // const _Rb_tree_node_base* parent() const { return _M_parent; }
    void parent(_Rb_tree_node_base* node) { _M_parent = node; }

    std::string to_string() const
    {
        switch (_M_color)
        {
        case _Rb_tree_color::_S_red: return "R";
        case _Rb_tree_color::_S_black: return "B";
        default: return "S";
        }
    }

    static constexpr void reset(_Rb_tree_node_base* node)
    {
        // node->_M_parent = nullptr;
        // node->_M_left = node->_M_right = node;
        node->_M_color  = _S_sentinel;
    }

    void as_empty_tree_header()
    {
        _Rb_tree_node_base::reset(this);
    }

    static constexpr void init(_Rb_tree_node_base* node)
    {
        // node->_M_left = node->_M_right = node->_M_parent = nullptr;
        node->_M_color  = _S_red;
    }

    void init()
    {
        _Rb_tree_node_base::init(this);
    }

    static constexpr bool is_header(const _Rb_tree_node_base *node)
    {
        return node->_M_color  == _S_sentinel;
    }

    bool is_header() const
    {
        return _Rb_tree_node_base::is_header(this);
    }

    static constexpr void clone(_Rb_tree_node_base *x, const _Rb_tree_node_base *y)
    {
        x->_M_color = y->_M_color ;
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
        __header._M_color  = _S_red;
        _Rb_tree_node_base *&__root = __header._M_parent;

        // Initialize fields in new node to insert.
        __x->_M_parent = __p;
        __x->_M_left = 0;
        __x->_M_right = 0;
        __x->_M_color  = _S_red;

        // Insert.
        // Make new node child of parent and maintain root, leftmost and
        // rightmost nodes.
        // N.B. First node is always inserted left.
        if (__insert_left)
        {
            __p->_M_left = __x; // also makes leftmost = __x when __p == &__header

            if (__p == &__header)
            {
                __header._M_parent = __x;
                __header._M_right = __x;
            }
            else if (__p == __header._M_left)
                __header._M_left = __x; // maintain leftmost pointing to min node
        }
        else
        {
            __p->_M_right = __x;

            if (__p == __header._M_right)
                __header._M_right = __x; // maintain rightmost pointing to max node
        }
        // Rebalance.
        while (__x != __root && __x->_M_parent->_M_color == _S_red)
        {
            _Rb_tree_node_base *const __xpp = __x->_M_parent->_M_parent;

            if (__x->_M_parent == __xpp->_M_left)
            {
                _Rb_tree_node_base *const __y = __xpp->_M_right;
                if (__y && __y->_M_color  == _S_red)
                {
                    __x->_M_parent->_M_color  = _S_black;
                    __y->_M_color  = _S_black;
                    __xpp->_M_color  = _S_red;
                    __x = __xpp;
                }
                else
                {
                    if (__x == __x->_M_parent->_M_right)
                    {
                        __x = __x->_M_parent;
                        tree_rotate_left(__x, __root);
                    }
                    __x->_M_parent->_M_color  = _S_black;
                    __xpp->_M_color  = _S_red;
                    tree_rotate_right(__xpp, __root);
                }
            }
            else
            {
                _Rb_tree_node_base *const __y = __xpp->_M_left;
                if (__y && __y->_M_color  == _S_red)
                {
                    __x->_M_parent->_M_color  = _S_black;
                    __y->_M_color  = _S_black;
                    __xpp->_M_color  = _S_red;
                    __x = __xpp;
                }
                else
                {
                    if (__x == __x->_M_parent->_M_left)
                    {
                        __x = __x->_M_parent;
                        tree_rotate_right(__x, __root);
                    }
                    __x->_M_parent->_M_color  = _S_black;
                    __xpp->_M_color  = _S_red;
                    tree_rotate_left(__xpp, __root);
                }
            }
        }
        __root->_M_color  = _S_black;
        __header._M_color  = _S_sentinel; // Make header black.
    }

    _Rb_tree_node_base* rebalance_for_erase(_Rb_tree_node_base& header)
    {
        return _Rb_tree_node_base::rebalance_for_erase(this, header);
    }

    static constexpr _Rb_tree_node_base* maximum(_Rb_tree_node_base* x)
    {
        assert(x && "x should not be nullptr");
        for (; x->_M_right; x = x->_M_right);
        return x;
    }

    _Rb_tree_node_base* minimum() { return _Rb_tree_node_base::minimum(this); }
    const _Rb_tree_node_base* minimum() const { return _Rb_tree_node_base::minimum(this); }

    _Rb_tree_node_base* maximum() { return _Rb_tree_node_base::maximum(this); }
    const _Rb_tree_node_base* maximum() const { return _Rb_tree_node_base::maximum(this); }

    static constexpr _Rb_tree_node_base* minimum(_Rb_tree_node_base* x)
    {
        assert(x && "x should not be nullptr");
        for (; x->_M_left; x = x->_M_left);
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
        _Rb_tree_node_base* y = x->_M_right;

        x->_M_right = y->_M_left;
        if (y->_M_left != 0)
            y->_M_left->_M_parent = x;
        y->_M_parent = x->_M_parent;

        // x->parent will never be nullptr, since header->parent == root and root->parent == header
        if (x == root)
            root = y;
        else if (x == x->_M_parent->_M_left) 
            x->_M_parent->_M_left = y;
        else
            x->_M_parent->_M_right = y;
        y->_M_left = x;
        x->_M_parent = y;
    }


    /*
    *     x        y                   
    *    /     =>    \
    *   y              x
    */
    static constexpr void tree_rotate_right(_Rb_tree_node_base* x, _Rb_tree_node_base*& root)
    {
        _Rb_tree_node_base* y = x->_M_left;

        x->_M_left = y->_M_right;
        if (y->_M_right != 0)
            y->_M_right->_M_parent = x;
        y->_M_parent = x->_M_parent;

        if (x == root)
            root = y;
        else if (x == x->_M_parent->_M_right)
            x->_M_parent->_M_right = y;
        else
            x->_M_parent->_M_left = y;
        y->_M_right = x;
        x->_M_parent = y;
    }


    static constexpr _Rb_tree_node_base *
    rebalance_for_erase(_Rb_tree_node_base *const __z,
                                    _Rb_tree_node_base &__header) 
    {

        __header._M_color  = _S_red;

        _Rb_tree_node_base *&__root = __header._M_parent;
        _Rb_tree_node_base *&__leftmost = __header._M_left;
        _Rb_tree_node_base *&__rightmost = __header._M_right;
        _Rb_tree_node_base *__y = __z;
        _Rb_tree_node_base *__x = 0;
        _Rb_tree_node_base *__x_parent = 0;

        if (__y->_M_left == 0)       // __z has at most one non-null child. y == z.
            __x = __y->_M_right;     // __x might be null.
        else if (__y->_M_right == 0) // __z has exactly one non-null child. y == z.
            __x = __y->_M_left;      // __x is not null.
        else
        {
            // __z has two non-null children.  Set __y to
            __y = __y->_M_right; //   __z's successor.  __x might be null.
            while (__y->_M_left != 0)
                __y = __y->_M_left;
            __x = __y->_M_right;
        }
        if (__y != __z)
        {
            // relink y in place of z.  y is z's successor
            __z->_M_left->_M_parent = __y;
            __y->_M_left = __z->_M_left;
            if (__y != __z->_M_right)
            {
                __x_parent = __y->_M_parent;
                if (__x)
                    __x->_M_parent = __y->_M_parent;
                __y->_M_parent->_M_left = __x; // __y must be a child of _M_left
                __y->_M_right = __z->_M_right;
                __z->_M_right->_M_parent = __y;
            }
            else
                __x_parent = __y;
            if (__root == __z)
                __root = __y;
            else if (__z->_M_parent->_M_left == __z)
                __z->_M_parent->_M_left = __y;
            else
                __z->_M_parent->_M_right = __y;
            __y->_M_parent = __z->_M_parent;
            std::swap(__y->_M_color , __z->_M_color );
            __y = __z;
            // __y now points to node to be actually deleted
        }
        else
        { // __y == __z
            __x_parent = __y->_M_parent;
            if (__x)
                __x->_M_parent = __y->_M_parent;
            if (__root == __z)
                __root = __x;
            else if (__z->_M_parent->_M_left == __z)
                __z->_M_parent->_M_left = __x;
            else
                __z->_M_parent->_M_right = __x;
            if (__leftmost == __z)
            {
                if (__z->_M_right == 0) // __z->_M_left must be null also
                    __leftmost = __z->_M_parent;
                // makes __leftmost == _M_header if __z == __root
                else
                    __leftmost = _Rb_tree_node_base::minimum(__x);
            }
            if (__rightmost == __z)
            {
                if (__z->_M_left == 0) // __z->_M_right must be null also
                    __rightmost = __z->_M_parent;
                // makes __rightmost == _M_header if __z == __root
                else // __x == __z->_M_left
                    __rightmost = _Rb_tree_node_base::maximum(__x);
            }
        }
        if (__y->_M_color  != _S_red)
        {
            while (__x != __root && (__x == 0 || __x->_M_color  == _S_black))
                if (__x == __x_parent->_M_left)
                {
                    _Rb_tree_node_base *__w = __x_parent->_M_right;
                    if (__w->_M_color  == _S_red)
                    {
                        __w->_M_color  = _S_black;
                        __x_parent->_M_color  = _S_red;
                        tree_rotate_left(__x_parent, __root);
                        __w = __x_parent->_M_right;
                    }
                    if ((__w->_M_left == 0 ||
                            __w->_M_left->_M_color  == _S_black) &&
                        (__w->_M_right == 0 ||
                            __w->_M_right->_M_color  == _S_black))
                    {
                        __w->_M_color  = _S_red;
                        __x = __x_parent;
                        __x_parent = __x_parent->_M_parent;
                    }
                    else
                    {
                        if (__w->_M_right == 0 || __w->_M_right->_M_color  == _S_black)
                        {
                            __w->_M_left->_M_color  = _S_black;
                            __w->_M_color  = _S_red;
                            tree_rotate_right(__w, __root);
                            __w = __x_parent->_M_right;
                        }
                        __w->_M_color  = __x_parent->_M_color ;
                        __x_parent->_M_color  = _S_black;
                        if (__w->_M_right)
                            __w->_M_right->_M_color  = _S_black;
                        tree_rotate_left(__x_parent, __root);
                        break;
                    }
                }
                else
                {
                    // same as above, with _M_right <-> _M_left.
                    _Rb_tree_node_base *__w = __x_parent->_M_left;
                    if (__w->_M_color  == _S_red)
                    {
                        __w->_M_color  = _S_black;
                        __x_parent->_M_color  = _S_red;
                        tree_rotate_right(__x_parent, __root);
                        __w = __x_parent->_M_left;
                    }
                    if ((__w->_M_right == 0 ||
                            __w->_M_right->_M_color  == _S_black) &&
                        (__w->_M_left == 0 ||
                            __w->_M_left->_M_color  == _S_black))
                    {
                        __w->_M_color  = _S_red;
                        __x = __x_parent;
                        __x_parent = __x_parent->_M_parent;
                    }
                    else
                    {
                        if (__w->_M_left == 0 || __w->_M_left->_M_color  == _S_black)
                        {
                            __w->_M_right->_M_color  = _S_black;
                            __w->_M_color  = _S_red;
                            //   tree_rotate_left(__w, __root);
                            tree_rotate_left(__w, __root);
                            __w = __x_parent->_M_left;
                        }
                        __w->_M_color  = __x_parent->_M_color ;
                        __x_parent->_M_color  = _S_black;
                        if (__w->_M_left)
                            __w->_M_left->_M_color  = _S_black;
                        //   tree_rotate_right(__x_parent, __root);
                        tree_rotate_right(__x_parent, __root);
                        break;
                    }
                }
            if (__x)
                __x->_M_color  = _S_black;
        }
        
        __header._M_color  = _S_sentinel;     // Is unnecessary?       
        return __y;
    }

};


using red_black_node = _Rb_tree_node_base;

} // namespace leviathan


