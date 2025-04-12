#pragma once

#include "value.hpp"

namespace cpp::config::toml::detail
{

// When we call emplace a default value, a temporary variable will always be 
// created. If some implementations of table cost two much during 
// its default construction, the operation will be slow.
// table->try_emplace(key, table());
struct table_maker
{
    constexpr explicit table_maker() = default;

    operator value() const
    {
        return table(false);
    }
};

struct array_maker
{
    constexpr explicit array_maker() = default;

    operator value() const
    {
        return array(false);
    }
};

inline value* try_put_value(std::vector<string> keys, value val, table* super)
{
    assert(super && "super cannot be nullptr.");
    
    [[assume(super != nullptr)]];

    bool ok = false;

    for (size_t i = 0; i < keys.size() - 1; ++i)
    {
        // When we call 'try_emplace', a temporary variable will always be 
        // created. If some implementations of table cost two much during 
        // its default construction, the operation will be slow.
        // auto [it, succeed] = super->try_emplace(std::move(keys[i]), table());
        auto [it, succeed] = super->try_emplace(std::move(keys[i]), table_maker());

        if (!succeed)
        {
            if (!it->second.is<table>())
            {
                throw_toml_parse_error("Key conflict");
            }
            if (it->second.as_ptr<table>()->is_locked())
            {
                throw_toml_parse_error("Inline table cannot be modified.");
            }
        }

        ok = ok || succeed;
        super = it->second.as_ptr<table>();
    }

    auto [it, succeed] = super->try_emplace(std::move(keys.back()), std::move(val));

    if (!succeed)
    {
        throw_toml_parse_error("Value already exits");
    }
    return &(it->second);
}

template <typename T>
inline bool is_unlocked(const toml::value& x)
{
    return x.is<T>() && !x.as<T>().is_locked();
}

inline value* insert_section(std::vector<string> keys, value* super, bool table_or_array)
{
    assert(super && "super cannot be nullptr.");
    [[assume(super != nullptr)]];

    for (size_t i = 0; i < keys.size() - 1; ++i)
    {
        auto [it, succeed] = super->as<table>().try_emplace(std::move(keys[i]), table_maker());

        if (succeed)
        {
            super = &(it->second);
        }
        else    
        {
            if (is_unlocked<array>(it->second))
            {
                super = &(it->second.as<array>().back());
            }
            else if (is_unlocked<table>(it->second))
            {
                super = &(it->second);
            }
            else    
            {
                throw_toml_parse_error("Key conflict.");
            }
        }
    }

    if (table_or_array)
    {
        auto [it, succeed] = super->as<table>().try_emplace(std::move(keys.back()), table_maker());
        
        if (succeed || is_unlocked<table>(it->second))
        {
            super = &(it->second);
        }
        else
        {
            throw_toml_parse_error("Table define conflict.");
        }
    }
    else
    {
        // array always ref the last value.
        if (super->is<array>())
        {
            super = &(super->as<array>().back());
        }
        auto [it, succeed] = super->as<table>().try_emplace(std::move(keys.back()), array_maker());

        if (succeed || is_unlocked<array>(it->second))
        {
            super = &(it->second.as<array>().emplace_back(table_maker()));
        }
        else
        {
            throw_toml_parse_error("Array define conflict.");
        }
    }

    return super;
}

} // namespace cpp::config::toml::detail

namespace cpp::config::toml
{

class collector
{
    // Root - TomlTable
    value m_global; 

    // For global, m_table point to m_global.
    // For std-table, m_table point to section node.
    // For array, m_table point to value of array.
    // The m_table must get memory during parsing array-table
    // and release after parsing.
    value* m_table;

public:

    collector() 
    {
        m_global = make_toml<table>(false);
        m_table = &m_global;
    }

    void switch_to_std_table(std::vector<string> section)
    {
        m_table = detail::insert_section(std::move(section), &m_global, true);
    }

    void switch_to_array_table(std::vector<string> section)
    {
        m_table = detail::insert_section(std::move(section), &m_global, false);
    }

    void add_entry(std::vector<string> keys, value x)
    {
        detail::try_put_value(std::move(keys), std::move(x), m_table->as_ptr<table>());
    }

    value dispose()
    {
        m_table = nullptr;
        return std::move(m_global);
    }
};


}

