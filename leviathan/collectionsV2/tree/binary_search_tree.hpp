#pragma once

#include "binary_node.hpp"
#include "tree.hpp"

namespace leviathan::collections
{

template <typename T, typename Compare = std::ranges::less, typename Allocator = std::allocator<T>>
using binary_search_tree = tree<identity<T>, Compare, Allocator, true, binary_node>;

}

