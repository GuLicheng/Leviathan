#pragma once

#include <leviathan/collections/common.hpp>
#include <leviathan/collections/hashtable/entry.hpp>

namespace leviathan::collections
{
    
template <typename KeyValue, 
    typename Hasher, 
    typename KeyEqual,
    typename Allocator,
    bool Unique = true>
class dictionary
{

    std::vector<size_t> m_indices;
    

};

} // namespace leviathan::collections

