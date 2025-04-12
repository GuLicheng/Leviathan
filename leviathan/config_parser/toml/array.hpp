#pragma once

#include <vector>

namespace cpp::config::toml::detail
{

// The locked array will inserted as value.
// The unlocked array will inserted as section.
template <typename T, typename Alloc>
class toml_array_base : public std::vector<T, Alloc>
{
    using base = std::vector<T, Alloc>;

    bool m_locked = false;  // for fixed array or table array

public:

    toml_array_base() = default;

    template <typename... Args>
    explicit toml_array_base(bool locked, Args&&... args) 
        : base((Args&&) args...), m_locked(locked) { }

    bool is_locked() const
    {
        return m_locked;
    }
};

}

