#pragma once

#include <leviathan/collections/common.hpp>

namespace cpp::collections
{
    
enum class entry_state : char
{
    uninitialized,
    deleted,
    active,
};

template <bool CacheHashCode = false>
struct entry
{
    entry_state m_state;
};

template <>
struct entry<true> : entry<false>
{
    std::size_t m_hash_code;
    entry_state m_state;
};

} // namespace cpp::collections

