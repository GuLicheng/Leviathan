#pragma once

#include "common.hpp"
#include "parse_context.hpp"
#include "toml_value2.hpp"
#include "../parser.hpp"

namespace leviathan::config::toml
{

struct toml_parse_error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

template <typename... Args>
[[noreturn]] void throw_toml_parse_error(std::string_view fmt, Args&&... args)
{
    auto msg = std::vformat(fmt, std::make_format_args(args...));
    throw toml_parse_error(msg);
}

template <typename... Args>
[[noreturn]] void check_and_throw(bool thrown, std::string_view fmt, Args&&... args)
{
    if (thrown)
    {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        throw toml_parse_error(msg);
    }
}

class decoder
{

public:
    parse_context m_ctx;

    decoder(std::string_view context) : m_ctx(context) { }

    void parse_section_or_entry()
    {
        while (!m_ctx.is_at_end())
        {
            auto line = ltrim(m_ctx.read_line());

            // Empty line or just commit.
            if (line.empty() || line.front() == '#')
            {   
                // U+0000 - U+0008 and U+000A - U+001F,U+007F should not appear in commit.
                continue;
            }
            if (line.front() == '[')
            {
                // [table] or [[table_array]]
                parse_section(line);
            }
            else
            {
                // parse key and value.
                parse_entry();
            }
        }
    }

    void parse_section(std::string_view line, table* global)
    {
        line.remove_prefix(1); // eat '['

        if (line.front() == '[')
        {
            parse_table_section(line, global);
        } 
        else
        {
            check_and_throw(line.size() && line.front() == '[', "table array should started with [[");
            parse_table_array_section(line, global);
        }
    }

    void parse_table_section(std::string_view line, table* super)
    {

    }

    void parse_entry(table* global, table* section_name)
    {
        
    }

};

} // namespace leviathan::config::toml

