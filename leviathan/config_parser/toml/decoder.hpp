/*
Reference:
    - https://mojotv.cn/2018/12/26/what-is-toml
    - https://toml.io/cn/v1.0.0
    - https://github.com/toml-lang/toml/blob/1.0.0/toml.abnf
    - https://github.com/BurntSushi/toml-test
*/

#pragma once

#include "value.hpp"
#include "../parse_context.hpp"

namespace leviathan::config::toml
{

namespace detail
{

// We define some constant values here.
inline constexpr std::string_view wschar = "\r ";
inline constexpr std::string_view ml_literal_string_delim = "'''";
inline constexpr std::string_view ml_basic_string_delim = "\"\"\"";
inline constexpr std::string_view array_table_open = "[[";
inline constexpr std::string_view array_table_close = "]]";

// For convenience, we keep linefeed be '\n' during reading file.
inline constexpr char newline = '\n';
inline constexpr char quotation_mark = '"';
inline constexpr char apostrophe = '\'';
inline constexpr char dot_sep = '.';
inline constexpr char escaped = '\\';
inline constexpr char keyval_sep = '=';
inline constexpr char comment_start_symbol = '#';
inline constexpr char std_table_open = '[';
inline constexpr char std_table_close = ']';
inline constexpr char array_open = '[';
inline constexpr char array_close = ']';
inline constexpr char array_sep = ',';
inline constexpr char inline_table_open = '{';
inline constexpr char inline_table_close = '}';
inline constexpr char inline_table_sep = ',';

template <size_t N, typename InputIterator>
void decode_unicode(InputIterator dest, parse_context& ctx) 
{
    if (ctx.size() != N)
    {
        throw_toml_parse_error("Too small characters for unicode");
    }

    auto codepoint = decode_unicode_from_char<N>(ctx.data());
    encode_unicode_to_utf8(dest, codepoint);
    ctx.advance_unchecked(N);
}

template <typename Fn>
string parse_basic_string_impl(parse_context& ctx, Fn fn)
{
    ctx.consume('\"');
    std::string out;

    while (ctx)
    {
        if (*ctx == detail::escaped)
        {
            ++ctx; // eat '\\'
            
            if (!ctx)
            {
                throw_toml_parse_error("Escaped cannot at the end.");
            }

            switch (*ctx)
            {
                case '"': out += '"'; break;    // quote
                case '\\': out += '\\'; break;  // reverse solidus
                case '/': out += '/'; break;    // solidus
                case 'b': out += '\b'; break;   // backspace
                case 'f': out += '\f'; break;   // formfeed
                case 'n': out += '\n'; break;   // linefeed
                case 'r': out += '\r'; break;   // carriage return
                case 't': out += '\t'; break;   // horizontal tab
                case 'u': decode_unicode<4>(std::back_inserter(out), ctx); break;
                case 'U': decode_unicode<8>(std::back_inserter(out), ctx); break;
                // TODO: \x -> two digits
                default: throw_toml_parse_error("Illegal character after escaped.");
            }
            ++ctx;
        }
        else if (*ctx == '\"') 
        {
            return out;
        }
        else
        {
            check_and_throw(fn(*ctx), "Invalid character {}.", *ctx);
            out += *ctx;
            ++ctx;
        }
    }

    throw_toml_parse_error("Basic string should end with \".");
}

} // namespace detail

class decoder
{
    enum kind { global, std_table, array_table };

    // Current status
    kind m_kind;

    // Context
    parse_context m_ctx;  

    // Root, always be table
    value m_global;         

    // Section names
    std::vector<string> m_table_keys;

    // Entry keys
    std::vector<string> m_entry_keys;

    static bool non_ascii(int ch)
    {
        return (0x80 <= ch && ch <= 0xD7FF)
            || (0xE000 <= ch && ch <= 0x10FFFF);
    }

public:

    decoder(std::string_view context) 
        : m_kind(kind::global),
          m_ctx(context),
          m_global(table()) { }

    value operator()()
    {
        parse();
        return std::move(m_global);
    }

    void parse()
    {
        parse_expression();
        
        while (!m_ctx.eof())
        {
            parse_newline();
            parse_expression();
        }
    }

    void parse_newline()
    {
        m_ctx.locate_character('\n');
    }

    void parse_wschar()
    {
        m_ctx.skip(detail::wschar);
    }

    void parse_expression()
    {
        while (!m_ctx.eof())
        {
            // Skip empty line.
            m_ctx.skip_whitespace(); 

            if (m_ctx.match(detail::std_table_open))
            {
                parse_table();
                // check_table_name();
            }
            else
            {
                auto [keys, v] = parse_keyval();
                
                if (m_kind == kind::std_table)
                {

                }
                else if (m_kind == kind::array_table)
                {

                }
                else
                {
                    // m_global.as<table>().
                }
            }
            parse_comment_optional();
            m_ctx.skip_whitespace(); 
        }
    }

    void try_add_value(value* super, std::vector<string> keys, value x)
    {
        [[assume(super != nullptr)]];

        if (super->is<table>())
        {
            // For global and std-table, super is table
            // and we collect all entries as paths 
        
        }
        else if (super->is<array>())
        {
            // For array-table, super is array, 
            // and we collect all entries a table
        }
        else
        {
            throw_toml_parse_error("What are you doing? Passing an error pointer?");
        }
    }

    // void try_put_value(std::vector<string> keys, toml_value&& val, toml_table* table = nullptr)
    // {
    //     bool ok = false;

    //     auto t = table ? table : m_cur_table;

    //     for (size_t i = 0; i < keys.size() - 1; ++i)
    //     {
    //         auto [it, succeed] = t->try_emplace(toml_string(keys[i]), toml_table());

    //         if (!succeed)
    //         {
    //             if (!it->second.is<toml_table>())
    //             {
    //                 throw_toml_parse_error("Key conflict");
    //             }
    //             if (it->second.as_ptr<toml_table>()->is_inline_table())
    //             {
    //                 throw_toml_parse_error("Inline table cannot be modified.");
    //             }
    //         }

    //         ok = ok || succeed;
    //         t = it->second.as_ptr<toml_table>();
    //     }

    //     if (!t->try_emplace(toml_string(keys.back()), std::move(val)).second)
    //     {
    //         throw_toml_parse_error("Value already exits");
    //     }
    // }

    void parse_table()
    {
        // Clear table and put new keys during parsing.
        m_table_keys.clear();

        if (m_ctx.match(detail::array_table_open))
        {
            parse_array_table();
            m_kind = kind::array_table;
        }
        else
        {
            parse_std_table();
            m_kind = kind::std_table;
        }
    }

    void parse_array_table()
    {
        m_ctx.consume(detail::array_table_open);
        parse_wschar();
        m_table_keys = parse_key();
        parse_wschar();
        check_and_throw(m_ctx.consume(detail::array_table_close), "Expected ]] after array table");
    }

    void parse_std_table()
    {
        m_ctx.consume(detail::std_table_open);
        parse_wschar();
        m_table_keys = parse_key();
        parse_wschar();
        check_and_throw(m_ctx.consume(detail::std_table_close), "Expected ] after std table.");
    }

    std::pair<std::vector<string>, value>
    parse_keyval()
    {
        auto keys = parse_key();
        parse_keyval_sep();
        auto value = parse_val();
        return std::make_pair(std::move(keys), std::move(value));
    }

    value parse_val()
    {
        if (m_ctx.eof())
        {
            throw_toml_parse_error("Expected value.");
        }

        switch (m_ctx.current())
        {
            case '[': return parse_array();
            case '{': return parse_inline_table();
            case 't': return parse_true();
            case 'f': return parse_false();
            case '"': return parse_basic_string_or_multiline();
            case '\'': return parse_literal_string_or_multiline();
            default: return parse_number_or_date_time();
        }
    }

    value parse_inline_table()
    {
        m_ctx.consume(detail::inline_table_open);
        parse_wschar();

        if (m_ctx.match(detail::inline_table_close))
        {
            return table();
        }
        else
        {
            table t;

            while (1)
            {
                auto [keys, v] = parse_keyval();
                // t.add_value(std::move(keys), std::move(v)); //  FIXME
                parse_wschar();

                if (m_ctx.match(detail::inline_table_close))
                {
                    return t;
                }
                else if (m_ctx.match(detail::inline_table_sep))
                {
                    m_ctx.advance_unchecked(1); // eat ','
                    continue;
                }
                throw_toml_parse_error("Expected ] or , after value.");                
            }
        }

        skip_ws_comment_newline();
        check_and_throw(m_ctx.consume(detail::inline_table_close), "Expected } after inline table.");
    }

    value parse_array()
    {
        m_ctx.consume(detail::array_open);
        skip_ws_comment_newline();

        array retval;

        if (m_ctx.match(detail::array_close))
        {
            m_ctx.advance_unchecked(1); // eat ']'
            return retval;
        }
        else
        {
            while (1)
            {
                retval.emplace_back(parse_val());
                skip_ws_comment_newline();

                if (m_ctx.match(detail::array_close))
                {
                    m_ctx.advance_unchecked(1);
                    return retval;
                }

                if (!m_ctx.match(detail::array_sep))
                {
                    throw_toml_parse_error("Expected ] after array.");
                }
                skip_ws_comment_newline();
            }
        }
    }

    void skip_ws_comment_newline()
    {
        // ws-comment-newline = *( wschar / [ comment ] newline )
        m_ctx.skip_whitespace();

        while (m_ctx.match(detail::comment_start_symbol))
        {
            parse_comment();
            m_ctx.skip_whitespace(); 
        }
    }

    value parse_number_or_date_time()
    {
        throw_toml_parse_error("Unimplemented.");
    }

    value parse_basic_string_or_multiline()
    {
        return m_ctx.match(detail::ml_basic_string_delim)
             ? parse_ml_basic_string()
             : parse_basic_string();
    }

    value parse_ml_basic_string()
    {
        throw_toml_parse_error("Not implemented");
    }

    value parse_literal_string_or_multiline()
    {
        return m_ctx.match(detail::ml_literal_string_delim)
             ? parse_ml_literal_string()
             : parse_literal_string();
    }

    value parse_ml_literal_string()
    {
        m_ctx.consume(detail::ml_literal_string_delim);
        string retval;
        check_and_throw(!m_ctx.eof(), "Expected literal body.");

        if (m_ctx.current() == detail::newline)
        {
            m_ctx.advance_unchecked(1);   
        }

        // If a line end with '\\' after trim right, the linefeed 
        // will be ignored.
        while (!m_ctx.eof())
        {
            const auto ch = m_ctx.current();
            
            if (ch == detail::apostrophe)
            {
                size_t count = 0;
                for (; !m_ctx.eof() && m_ctx.current() == detail::apostrophe; ++count);
                check_and_throw(count < 6, "Too much quote.");
        
                if (count >= 3)
                {
                    retval.append(count - 3, detail::apostrophe);
                    m_ctx.advance_unchecked(count - 3);
                    m_ctx.consume(detail::ml_literal_string_delim);
                    return retval;
                }
                else
                {
                    retval.append(count, detail::apostrophe);
                    m_ctx.advance_unchecked(count);
                }
            }
            else if (ch == '\\')
            {
                auto view = m_ctx.slice(1);
                size_t i = 0;
                for (; i < view.size() && detail::wschar.contains(view[i]); ++i);
                check_and_throw(i < view.size(), "Expected ''' after multiline literal string.");
                
                // Check whether current line is end with '\\' after trim.
                // Since the index is started with 0, And the first 
                // character is '\\', so we add two.
                auto offset = i + 2;
                
                if (view[i] == detail::newline)
                {
                    m_ctx.advance_unchecked(offset);
                }
                else
                {
                    retval += m_ctx.slice(0, offset);
                    m_ctx.advance_unchecked(offset);
                }
            }
            else
            {
                retval += m_ctx.current();
                m_ctx.advance_unchecked(1);
            }
        }

        throw_toml_parse_error("Expected ''' after multiline literal string.");
    }

    value parse_true()
    {
        check_and_throw(
            m_ctx.match_literal_and_advance("true"),
            "Boolean literal should be one of [true, false]."
        );
        return boolean(true);
    }

    value parse_false()
    {
        check_and_throw(
            m_ctx.match_literal_and_advance("false"),
            "Boolean literal should be one of [true, false]."
        );
        return boolean(false);
    }

    void parse_keyval_sep()
    {
        parse_wschar();
        check_and_throw(m_ctx.consume(detail::keyval_sep), "Expected keyval-sep: =");
        parse_wschar();
    }

    std::vector<string> parse_key()
    {
        std::vector<string> keys;

        keys.emplace_back(parse_simple_key());

        while (parse_dot_sep())
        {
            keys.emplace_back(parse_simple_key());
        }
        return keys;
    }

    bool parse_dot_sep()
    {
        parse_wschar();

        if (m_ctx.match(detail::dot_sep))
        {
            m_ctx.advance_unchecked(1);
            parse_wschar();
            return true;
        }
        return false;
    }

    string parse_simple_key()
    {
        if (m_ctx.match(detail::apostrophe))
        {
            return parse_literal_string();
        }
        else if (m_ctx.match(detail::escaped))
        {
            return parse_basic_string();
        }
        else
        {
            return parse_unquoted_key();
        }
    }

    string parse_literal_string()
    {
        m_ctx.consume(detail::apostrophe);
        // literal-char = %x09 / %x20-26 / %x28-7E / non-ascii
        auto literal_character = [](size_t ch) static {
            return ch == 0x09
                || (0x20 <= ch && ch <= 0x26)
                || (0x28 <= ch && ch <= 0x7E)
                || non_ascii(ch);
        };

        string retval;

        for (; m_ctx && literal_character(*m_ctx); ++m_ctx)
        {
            retval += *m_ctx;
        }
        check_and_throw(m_ctx.consume(detail::apostrophe), "literal string must end with '.");
        return retval;
    }

    string parse_basic_string()
    {
        m_ctx.consume(detail::quotation_mark);
        auto basic_unescaped = [](int ch) static {
            return ch == 0x21
                || ch == ' '
                || ch == '\t'
                || (0x23 <= ch && ch <= 0x5B)
                || (0x5D <= ch && ch <= 0x7E)
                || non_ascii(ch);
        };
        string retval = detail::parse_basic_string_impl(m_ctx, basic_unescaped);
        check_and_throw(m_ctx.consume(detail::quotation_mark), "literal string must end with \".");
        return retval;
    }

    string parse_unquoted_key()
    {
        string retval;
        auto unquoted_key_char = [](int ch) static {
            static std::string_view special = "\x2D\x5F\xB2\xB3\xB9";
            return ::isalnum(ch) 
                || special.contains(ch); //  FIXME
        };

        for (; m_ctx && unquoted_key_char(*m_ctx); ++m_ctx)
        {
            retval += *m_ctx;
        }
        return retval;
    }

    // FIXME
    void parse_comment_optional()
    {
        parse_wschar();

        if (m_ctx.is_newline())
        {
            // m_ctx.skip(detail::newline);
            m_ctx.advance_unchecked(1);
        }
        else
        {
            if (!m_ctx.match(detail::comment_start_symbol))
            {
                throw_toml_parse_error("comment should start with #, but got {}.", m_ctx.current());
            }
            parse_comment();
        }
    }

    void parse_comment()
    {
        m_ctx.consume(detail::comment_start_symbol);
        m_ctx.locate_character(detail::newline);
        m_ctx.advance_unchecked(1);
    }
};

inline value loads(std::string source)
{
    return decoder(source)();
}

inline value load(const char* filename)
{
    // Windows
    constexpr const char* linefeed = "\r\n";
    return loads(leviathan::read_file_context(filename, linefeed));
}

} // namespace leviathan::config::toml



