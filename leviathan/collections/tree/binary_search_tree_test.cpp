#include <catch2/catch_all.hpp>

#include "binary_search_tree.hpp"

using namespace cpp::collections;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using Tree = tree_set<binary_node, T, Compare, Allocator>;

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using TreeWithMultiKey = tree_multiset<binary_node, T, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using TreeMap = tree_map<binary_node, K, V, Compare, Allocator>;

template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
using TreeMultiMap = tree_multimap<binary_node, K, V, Compare, Allocator>;

template <typename T, typename Alloc>
using TreeWithAlloc = tree<::identity<T>, std::ranges::less, Alloc, true, binary_node>;

// using Tree = avl_set<int>;

#include "tree_test.inc"










