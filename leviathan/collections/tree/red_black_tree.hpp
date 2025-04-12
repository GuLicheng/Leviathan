#pragma once

// #include "red_black_node_from_stlibc++.hpp"
#include "red_black_node.hpp"
#include "tree.hpp"

namespace cpp::collections
{
    
template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using red_black_treeset = tree_set<red_black_node, T, Compare, Allocator>;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using red_black_tree_multiset = tree_multiset<red_black_node, T, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using red_black_treemap = tree_map<red_black_node, K, V, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using red_black_tree_multimap = tree_multimap<red_black_node, K, V, Compare, Allocator>;

} // namespace cpp::collections

