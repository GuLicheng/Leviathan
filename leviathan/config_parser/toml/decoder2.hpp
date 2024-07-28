#pragma once

#include "toml_value.hpp"
#include "../parse_context.hpp"

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

    void read_next_line()
    {
        m_line = m_ctx.read_line();
    }

    void consume(std::string_view context, std::string_view message = "")
    {
        check_and_throw(m_line.consume(context), message);
    }

    void consume(char ch, std::string_view message = "")
    {
        check_and_throw(m_line.consume(ch), message);
    }

    static bool non_ascii(char ch)
    {
        return (0x80 <= ch && ch <= 0xD7FF)
            || (0xE000 <= x && x <= 0x10FFFF);
    }

public:
    parse_context m_ctx;
    parse_line m_line;

    decoder(std::string_view context) : m_ctx(context) { }

    void parse()
    {
        parse_expression();
    }

    void parse_expression()
    {
        while (!m_ctx.is_at_end())
        {
            read_next_line();
            m_line.trim_left();

            // expression =  ws [ comment ]
            if (m_line.empty())
            {
                continue;
            }

            const auto ch = m_line.current();

            if (ch == '#')
            {
                // U+0000 - U+0008 and U+000A - U+001F,U+007F should not appear in commit.
                continue; 
            }
            else if (ch == '[')
            {
                // expression =/ ws table ws [ comment ]
                parse_table();
            }
            else
            {
                // expression =/ ws keyval ws [ comment ]
                parse_keyval();
            }
        }
    }

    void parse_table()
    {
        consume('['); // eat '['
        check_and_throw(!line.empty(), "Only left bracket.");

        // table = std-table / array-table
        if (m_line.current() == '[')
        {
            parse_array_table();
        }
        else
        {
            parse_std_table();
        }

        // parse_keyval(); ??

        // skip_comment();
    }

    void skip_comment()
    {
        consume('#');
        read_next_line(); // FIXME
    }

    void parse_array_table()
    {
        // array-table = array-table-open key array-table-close
        consume("[", "Array table should started with [[");
        parse_key();
        consume("]]", "Array table should ends with ]]");
        skip_comment();
    }

    void parse_key()
    {
        // key = simple-key / dotted-key
        // dotted-key = simple-key 1*( dot-sep simple-key )

        parse_simple_key();
        
        while (m_line.size() && m_line.current() == '.')
        {
            
        }

        do
        {
            parse_simple_key();
            m_line.trim_left();
        } while (m_line.size() && m_line.current() == '.');
    }

    void parse_simple_key()
    {
        // simple-key = quoted-key / unquoted-key
        m_line.trim_left();

        if (m_line.empty())
        {
            return;
        }

        const auto ch = m_line.current();

        if (ch == '"')
        {
            parse_basic_string();
        }
        else if (ch == '\'')
        {
            parse_literal_string();
        }
        else
        {
            parse_unquoted_key();
        }
    }

    void parse_basic_string()
    {

    }

    void parse_literal_string()
    {
        // literal-char = %x09 / %x20-26 / %x28-7E / non-ascii
        auto valid_character = [](size_t n) static {
            return x == 0x09
                || (0x20 <= x && x <= 0x26)
                || (0x28 <= x && x <= 0x7E)
                || non_ascii(n);
        };

        while (m_line.size() && valid_character(m_line.current()))
        {
            m_line.advance_unchecked(1);
        }

        check_and_throw(m_line.size(), "Expected ' after literal_string.");
        consume('\'', "Expected ' after literal_string.");
    }

    void parse_std_table()
    {
        parse_key();
        consume(']', "Expected ']' after table.");
        skip_comment();
    }

    void parse_keyval();


};

} // namespace leviathan::config::toml



