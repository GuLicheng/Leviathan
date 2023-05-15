
#include <lv_cpp/collections/internal/binary_search_tree.hpp>

using leviathan::collections::simple_binary_node;
using leviathan::collections::binary_set;
using leviathan::collections::binary_map;
using SetT = binary_set<int>;
using MapT = binary_map<int, std::string>;
using TreeNodeT = simple_binary_node;


#include "test_tree.ixx"


#include "test_container_allocator.hpp"

// template <typename T, typename Compare = std::less<>, typename Allocator = std::allocator<T>>
// using binary_set = tree_set<T, Compare, Allocator, simple_binary_node>;


template <typename T, typename Allocator>
using TreeT = leviathan::collections::binary_set<
    T, std::less<>, Allocator
>;

using TrackedT = tracked<int>;

CreatePropagateTestingWithAllocator(TreeT, checked_allocator, TrackedT)
