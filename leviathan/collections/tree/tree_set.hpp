#pragma once

#include "tree.hpp"

namespace leviathan::collections
{
    
template <typename T, 
    typename Node, 
    typename Compare = std::ranges::less, 
    typename Allocator = std::allocator<T>>
using tree_set = tree<identity<T>, Compare, Allocator, true, Node>;


} // namespace leviathan::collections

