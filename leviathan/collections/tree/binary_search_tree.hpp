#pragma once

#include "binary_node.hpp"
#include "tree.hpp"

namespace leviathan::collections
{

template <typename T, typename Compare = std::ranges::less, typename Allocator = std::allocator<T>>
using binary_search_tree = tree<identity<T>, Compare, Allocator, true, binary_node>;

template <typename K, typename V, typename Compare = std::ranges::less, typename Allocator = std::allocator<std::pair<K, V>>>
using binary_search_tree_map = tree_map<K, V, Compare, Allocator, true, binary_node>;

}

