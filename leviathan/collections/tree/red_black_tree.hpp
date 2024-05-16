#pragma once

#include "../common.hpp"
#include "red_black_node.hpp"
#include "tree.hpp"

namespace leviathan::collections
{
    
template <typename T, typename Compare = std::ranges::less, typename Allocator = std::allocator<T>>
using red_black_tree_set = tree<identity<T>, Compare, Allocator, true, red_black_node>;



} // namespace leviathan::collections

