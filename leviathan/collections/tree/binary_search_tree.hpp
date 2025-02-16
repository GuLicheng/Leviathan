#pragma once

#include "binary_node.hpp"
#include "tree.hpp"

namespace leviathan::collections
{
    
template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
using binary_search_treeset = tree_set<binary_node, T, Compare, Allocator>;

template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
using binary_search_tree_multiset = tree_multiset<binary_node, T, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<>, typename Allocator = std::allocator<std::pair<const K, V>>>
using binary_search_treemap = tree_map<binary_node, K, V, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<>, typename Allocator = std::allocator<std::pair<const K, V>>>
using binary_search_tree_multimap = tree_multimap<binary_node, K, V, Compare, Allocator>;

} // namespace leviathan::collections

