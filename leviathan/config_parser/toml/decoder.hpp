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

namespace detail
{

// We define some constant values here.
inline constexpr std::string_view wschar = "\r ";
inline constexpr std::string_view ml_literal_string_delim = "'''";
inline constexpr std::string_view ml_basic_string_delim = "\"\"\"";

// For convenience, we keep linefeed be '\n' during reading file.
inline constexpr char newline = '\n';
inline constexpr char quotation_mark = '"';
inline constexpr char apostrophe = '\'';
inline constexpr char dot_sep = '.';
inline constexpr char escaped = '\\';

template <typename InputIterator, size_t N>
void decode_unicode(InputIterator dest, parse_context& ctx) 
{
    if (sv.size() != N)
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

            switch (&ctx)
            {
                case '"': out += '"'; break;    // quote
                case '\\': out += '\\'; break;  // reverse solidus
                case '/': out += '/'; break;    // solidus
                case 'b': out += '\b'; break;   // backspace
                case 'f': out += '\f'; break;   // formfeed
                case 'n': out += '\n'; break;   // linefeed
                case 'r': out += '\r'; break;   // carriage return
                case 't': out += '\t'; break;   // horizontal tab
                case 'u': decode_unicode<4>(std::back_inserter(out), ctx.slice(1, 4)); break;
                case 'U': decode_unicode<8>(std::back_inserter(out), ctx.slice(1, 8)); break;
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
    parse_context m_ctx;
    table m_global; 
    std::vector<string> m_section_names;
    std::vector<string> m_key_names;

    static bool non_ascii(char ch)
    {
        return (0x80 <= ch && ch <= 0xD7FF)
            || (0xE000 <= x && x <= 0x10FFFF);
    }

public:

    decoder(std::string_view context) : m_ctx(context) { }

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
        m_ctx.skip(wschar);
    }

    void parse_expression()
    {
        parse_wschar();

        if (m_ctx.match('['))
        {
            parse_table();
        }
        else
        {
            parse_keyval();
        }
        parse_comment_optional();
    }

    void parse_keyval()
    {
        parse_key();
        parse_keyval_sep();
        parse_val();
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

    value parse_number_or_date_time();

    value parse_basic_string_or_multiline();

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

    void parse_key()
    {
        parse_simple_key();

        while (parse_dot_sep())
        {
            parse_simple_key();
        }
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
                || non_ascii(n);
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
        auto basic_unescaped = [](char ch) static {
            return ch == 0x21
                || ch == ' '
                || ch == '\t'
                || (0x23 <= ch && ch <= 0x5B)
                || (0x5D <= ch && ch <= 0x7E)
                || non_ascii(n);
        };
        string retval = detail::parse_basic_string_impl(m_ctx, basic_unescaped);
        check_and_throw(m_ctx.consume(detail::quotation_mark), "literal string must end with \".");
    }

    string parse_unquoted_key()
    {
        string retval;
        auto unquoted_key_char = [](char ch) static {
            static std::string_view special = "\x2D\x5F\xB2\xB3\xB9";
            return ::isalnum(ch) 
                || special.contains(ch); //  FIXME
        };

        for (; m_ctx; ++m_ctx)
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
            m_ctx.skip(newline);
        }
        else
        {
            if (!m_ctx.match('#'))
            {
                throw_toml_parse_error("comment should start with #, but got {}.", m_ctx.current());
            }
            m_ctx.consume('#');
            m_ctx.locate_character('\n');
            m_ctx.advance_unchecked(1);
        }
    }
};


} // namespace leviathan::config::toml



