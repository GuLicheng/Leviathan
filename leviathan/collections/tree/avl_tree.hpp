#pragma once

#include "avl_node.hpp"
#include "tree.hpp"

namespace leviathan::collections
{
    
template <typename T, typename Compare = std::ranges::less, typename Allocator = std::allocator<T>>
using avl_tree = tree<identity<T>, Compare, Allocator, true, avl_node>;



} // namespace leviathan::collections

