#pragma once

#include <unordered_map>

namespace leviathan::config::toml::detail
{
    
template <typename K, typename V, typename HasherKeyEqual>
class toml_table_base : public std::unordered_map<K, V, HasherKeyEqual, HasherKeyEqual>
{
    using base = std::unordered_map<K, V, HasherKeyEqual, HasherKeyEqual>;

    bool m_locked = false;   // for table or inline table

    bool m_defined = false;  // for defining a super-table afterward 

public:

    toml_table_base() = default;

    template <typename... Args>
    explicit toml_table_base(bool locked, Args&&... args) 
        : base((Args&&) args...), m_locked(locked) { }

    bool is_inline_table() const
    {
        return m_locked;
    }

    bool is_table() const
    {
        return !m_locked;
    }

    bool is_defined() const
    {
        return m_defined;
    }

    void define_table()
    {
        m_defined = true;
    }
};

} // namespace leviathan::config::toml::detail
