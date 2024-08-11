#pragma once

#include "value.hpp"

namespace leviathan::config::toml
{
    
inline value* try_generate_path(std::vector<string> keys, value val, table* super)
{
    assert(super && "super cannot be nullptr.");
    
    [[assume(super != nullptr)]];

    bool ok = false;

    for (size_t i = 0; i < keys.size() - 1; ++i)
    {
        // When we call 'try_emplace', a temporary variable will always be 
        // created. If some implementation of table cost two much during 
        // its default construction, the operation will be slow.
        // auto [it, succeed] = super->try_emplace(std::move(keys[i]), table());
        auto [it, succeed] = super->try_emplace((keys[i]), table());

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

    // auto [it, succeed] = super->try_emplace(std::move(keys.back()), std::move(val));
    auto [it, succeed] = super->try_emplace((keys.back()), std::move(val));

    if (!succeed)
    {
        throw_toml_parse_error("Value already exits");
    }
    return std::addressof(it->second);
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

    // Point to the array table, null when parsing std-table and global.
    value* m_array;
    
    enum { std_table, array_table, global } m_mode;

public:

    collector() 
    {
        m_global = make_toml<table>();
        m_mode = global; 
        m_table = &m_global;
        m_array = nullptr;
    }

    void switch_to_std_table(std::vector<string> section)
    {
        collect();
        m_mode = std_table;
        m_table = try_generate_path(std::move(section), table(), m_global.as_ptr<table>());
    }

    void switch_to_array_table(std::vector<string> section)
    {
        collect();
        m_mode = array_table;
        m_array = try_generate_path(std::move(section), array(), m_global.as_ptr<table>());
        m_table = new value(table());
    }

    void add_entry(std::vector<string> keys, value x)
    {
        try_generate_path(std::move(keys), std::move(x), m_table->as_ptr<table>());
    }

    void collect()
    {
        if (m_mode == array_table)
        {
            m_array->as<array>().emplace_back(std::move(m_table->as<table>()));
            m_array = nullptr;    
            m_table->as<table>().clear();
            delete m_table;
        }
    }

    table dispose()
    {
        return std::move(m_global.as<table>());
    }
};

} // namespace leviathan::config::toml

