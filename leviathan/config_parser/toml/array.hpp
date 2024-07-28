#pragma once

#include <vector>

namespace leviathan::config::toml::detail
{

template <typename T>
class toml_array_base : public std::vector<T>
{
    using base = std::vector<T>;

    bool m_locked = false;  // for fixed array or table array

public:

    toml_array_base() = default;

    template <typename... Args>
    explicit toml_array_base(bool locked, Args&&... args) 
        : base((Args&&) args...), m_locked(locked) { }

    bool is_table_array() const 
    { return !m_locked; }

    bool is_array() const
    { return m_locked; }
};

}

