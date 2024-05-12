#pragma once

#include "tree_node.hpp"

#include <algorithm>


#include "tree.hpp"

namespace leviathan::collections
{
    
template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
using avl_set = tree_set<T, Compare, Allocator, avl_node>;

template <typename T, typename Compare = std::less<>>
using pmr_avl_set = tree_set<T, Compare, std::pmr::polymorphic_allocator<T>, avl_node>;  

template <typename K, typename V, typename Compare = std::less<>, typename Allocator = std::allocator<std::pair<const K, V>>>
using avl_map = tree_map<K, V, Compare, Allocator, avl_node>;

template <typename K, typename V, typename Compare = std::less<>>
using pmr_avl_map = tree_map<K, V, Compare, std::pmr::polymorphic_allocator<std::pair<const K, V>>, avl_node>;

}
