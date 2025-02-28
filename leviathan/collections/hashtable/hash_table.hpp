#pragma once

#include "../common.hpp"

namespace leviathan::collections
{
    
template <typename KeyValue, typename Hasher, typename KeyEqual, typename Allocator, typename Slot>
class hash_table
{
    static constexpr bool IsTransparent = detail::transparent<hasher, key_equal>;
    using slot_index_type = std::size_t;

public:

    using key_type = typename KeyValue::key_type;
    using value_type = typename KeyValue::value_type;
    using hasher = Hasher;
    using key_equal = KeyEqual;
    using allocator_type = Allocator;

    

protected:

    hash_key_equal<hasher, key_equal> m_hk;
    Slot m_slots;
    [[no_unique_address]] Allocator m_alloc;
};

} // namespace leviathan::collections

