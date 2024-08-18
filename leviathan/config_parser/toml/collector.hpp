#pragma once

#include "value.hpp"

namespace leviathan::config::toml
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
        return table();
    }
};

inline void remove_empty_table(array& arr)
{
    std::erase_if(arr, [](const value& x)
    {
        return x.is<table>() && x.as<table>().empty();
    });
}    

inline value* try_generate_path(std::vector<string> keys, value val, table* super)
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
            if (it->second.as_ptr<table>()->is_inline_table())
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

inline value* generate_section_path(std::vector<string> keys, value val, value* super)
{
    assert(super && "super cannot be nullptr.");
    [[assume(super != nullptr)]];

    for (size_t i = 0; i < keys.size(); ++i)
    {
        auto [it, succeed] = super->as<table>().try_emplace(std::move(keys[i]), table_maker());

        if (succeed)
        {
            super = &(it->second);
        }
        else
        {
            if (it->second.is<array>() && it->second.as<array>().is_table_array())
            {
                super = &(it->second.as<array>().emplace_back(table_maker()));
                super->as<table>().emplace(std::move(keys[i]), table_maker());
                // remove_empty_table(it->second.as<array>());
            }
            else if (it->second.is<table>())
            {
                if (it->second.as<table>().is_inline_table())
                {
                    throw_toml_parse_error("Inline table cannot be modified.");
                }
                else
                {
                    super = &(it->second);
                }
            }
            else
            {
                throw_toml_parse_error("Key conflict");
            }
        }
    }

    if (val.is<array>())
    {
        *super = std::move(val);
    }

    return super;
}

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

    enum { std_table, array_table, global } m_mode;

public:

    collector() 
    {
        m_global = make_toml<table>();
        m_mode = global; 
        m_table = &m_global;
    }

    void switch_to_std_table(std::vector<string> section)
    {
        m_mode = std_table;
        m_table = generate_section_path(std::move(section), table(), &m_global);
    }

    void switch_to_array_table(std::vector<string> section)
    {
        m_mode = array_table;
        auto parr = generate_section_path(std::move(section), array(), &m_global);
        m_table = &(parr->as<array>().emplace_back(table_maker()));
    }

    void add_entry(std::vector<string> keys, value x)
    {
        try_generate_path(std::move(keys), std::move(x), m_table->as_ptr<table>());
    }

    value dispose()
    {
        m_table = nullptr;
        return std::move(m_global);
    }
};

} // namespace leviathan::config::toml

