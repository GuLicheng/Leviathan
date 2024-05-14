#include <iostream>
#include <leviathan/meta/template_info.hpp>

#include "catch2/catch_all.hpp"
#include "binary_search_tree.hpp"

using namespace leviathan::collections;

using Tree = binary_search_tree<int>; 

template <typename T, typename Alloc>
using TreeWithAlloc = tree<identity<T>, std::ranges::less, Alloc, true, binary_node>;

template <typename K, typename V>
using TreeMap = binary_search_tree_map<K, V>;

#include "tree_test.inc"


