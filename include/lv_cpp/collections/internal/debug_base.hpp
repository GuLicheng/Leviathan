#pragma once

namespace leviathan::debug
{

    // debug for tree
    template <typename Tree>
    struct tree_traits;

    /*
    template <typename Tree>
    struct tree_traits
    {
        // Tree::node_type sometimes will return node_handle
        // which is the return_type of extract.
        using node_type = typename Tree::tree_node;

        // Convert node to string
        static std::string to_string(node_type*);

        // Return left child of x
        static node_type* left(node_type* x);

        // Return right child of x
        static node_type* right(node_type* x);

        // Return root of tree
        static node_type* root(Tree* t);
    };
    */

}