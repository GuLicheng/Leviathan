#pragma once

#include "treap_node.hpp"
#include "tree.hpp"

namespace cpp::collections
{

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using treap_set = tree_set<default_treap_node, T, Compare, Allocator>;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using treap_multiset = tree_multiset<default_treap_node, T, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using treap_map = tree_map<default_treap_node, K, V, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using treap_multimap = tree_multimap<default_treap_node, K, V, Compare, Allocator>;

}  // namespace cpp::collections
