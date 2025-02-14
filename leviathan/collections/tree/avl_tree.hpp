#pragma once

#include "avl_node.hpp"
#include "tree.hpp"

namespace leviathan::collections
{
    
template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
using avl_treeset = tree_set<avl_node, T, Compare, Allocator>;

template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
using avl_tree_multiset = tree_multiset<avl_node, T, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<>, typename Allocator = std::allocator<std::pair<const K, V>>>
using avl_treemap = tree_map<avl_node, K, V, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<>, typename Allocator = std::allocator<std::pair<const K, V>>>
using avl_tree_multimap = tree_multimap<avl_node, K, V, Compare, Allocator>;

} // namespace leviathan::collections

