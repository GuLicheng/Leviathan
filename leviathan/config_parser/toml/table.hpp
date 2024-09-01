#pragma once

#include <unordered_map>

namespace leviathan::config::toml::detail
{
    
// The locked table will inserted as value.
// The unlocked table will inserted as section.
template <typename K, typename V, typename HasherKeyEqual>
class toml_table_base : public std::unordered_map<K, V, HasherKeyEqual, HasherKeyEqual>
{
    using base = std::unordered_map<K, V, HasherKeyEqual, HasherKeyEqual>;

    bool m_locked = false;   // for table or inline table

public:

    using base::base;  // necessary for std::ranges::to 

    toml_table_base() = default;

    explicit toml_table_base(bool locked) : base(), m_locked(locked) { }

    bool is_locked() const
    {
        return m_locked;
    }
};

} // namespace leviathan::config::toml::detail
