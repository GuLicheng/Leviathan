/*
Reference:
    - https://mojotv.cn/2018/12/26/what-is-toml
    - https://toml.io/cn/v1.0.0
    - https://github.com/toml-lang/toml/blob/1.0.0/toml.abnf
    - https://github.com/BurntSushi/toml-test
*/

#pragma once

#include <leviathan/config_parser/parse_context.hpp>
#include <leviathan/config_parser/toml/value.hpp>
#include <leviathan/config_parser/toml/collector.hpp>
#include <leviathan/config_parser/toml/parser_helper.hpp>
#include <leviathan/type_caster.hpp>

namespace cpp::config::toml
{

namespace detail
{

template <typename Context>
struct toml_string_decoder
{
    template <size_t N, typename InputIterator>
    static void decode_unicode(InputIterator dest, Context& ctx) 
    {
        assert(ctx[0] == 'u' || ctx[0] == 'U');
        ctx.advance(1);

        if (ctx.size() < N)
        {
            throw toml_parse_error("Too small characters for unicode");
        }

        auto codepoint = decode_unicode_from_char<N>(ctx.begin());
        encode_unicode_to_utf8(dest, codepoint);
        ctx.advance(N);
    }

    static bool valid_simple_key_character(char ch)
    {
        return (ch >= 'a' && ch <= 'z')
            || (ch >= 'A' && ch <= 'Z')
            || (ch >= '0' && ch <= '9')
            || ch == '-'
            || ch == '_';
    }

    static string decode_literal_string(Context& ctx)
    {
        if (!ctx.match('\'', true))
        {
            throw toml_parse_error("Literal string must start with '.");
        }

        auto sv = ctx.take_while([](char ch) static { return ch != '\''; });
        ctx.advance(sv.size()); 

        if (!ctx.match('\'', true))
        {
            throw toml_parse_error("Literal string must end with '.");
        }

        return string(sv.begin(), sv.end());
    }

    static string decode_basic_string(Context& ctx)
    {
        if (!ctx.match('"', true))
        {
            throw toml_parse_error("Basic string must start with \".");
        }

        string result;

        auto clone = ctx;

        while (!ctx.eof())
        {
            if (ctx.match('"', true))
            {
                return result;
            }
            else if (ctx.current() == '\\')
            {
                ctx.advance(1); // eat '\'

                if (ctx.eof())
                {
                    ctx = std::move(clone);
                    throw toml_parse_error("Invalid escape sequence at end of basic string.");
                }

                switch (ctx.current())
                {
                    case 'b': result += '\b'; ctx.advance(1); break;
                    case 't': result += '\t'; ctx.advance(1); break;
                    case 'n': result += '\n'; ctx.advance(1); break;
                    case 'f': result += '\f'; ctx.advance(1); break;
                    case 'r': result += '\r'; ctx.advance(1); break;
                    case '"': result += '"'; ctx.advance(1); break;
                    case '\\': result += '\\'; ctx.advance(1); break;
                    case 'u': decode_unicode<4>(std::back_inserter(result), ctx); break;
                    case 'U': decode_unicode<8>(std::back_inserter(result), ctx); break;
                    // TODO: \x -> two digits
                    default: throw toml_parse_error("Illegal character after \\");
                }
            }
            else
            {
                result += ctx.current();
                ctx.advance(1);
            }
        }
    }

    static string decode_multiline_literal_string(Context& ctx);

    static string decode_multiline_basic_string(Context& ctx);

    static string decode_unquote_string(Context& ctx)
    {
        auto sv = ctx.take_while(valid_simple_key_character);

        if (sv.empty())
        {
            throw toml_parse_error("Unquote string must not be empty.");
        }

        ctx.advance(sv.size());
        return string(sv.begin(), sv.end());
    }

    static string decode_simple_key(Context& ctx)
    {
        if (ctx.current() == '\'')
        {
            return decode_literal_string(ctx);
        }
        else if (ctx.current() == '"')
        {
            return decode_basic_string(ctx);
        }
        else if (valid_simple_key_character(ctx.current()))
        {
            return decode_unquote_string(ctx);
        }
        else
        {
            throw toml_parse_error("Invalid simple key start character");
        }
    }
};

template <typename Context>
struct toml_number_decoder
{
    using char_type = typename Context::value_type;

    static bool valid_number_character(char_type ch)
    {
        // +-.eE_ [0-9|0x|0o|0b|inf|nan]
        static std::basic_string_view<char_type> valid_chars = "+-.eE_xobin";
        return ::isxdigit(ch) || valid_chars.contains(ch);
    }

    static string extract_number_string_and_remove_underscore(const Context& ctx)
    {
        auto sv = ctx.take_while(valid_number_character);
        // C++ allows ' in number literal since C++14, we convert '_' to '\'' 
        // and use std::from_chars to help us.
        auto underscore_to_quote = [](char_type ch) static { return ch == '_' ? '\'' : ch; };
        return sv | std::views::transform(underscore_to_quote) | std::ranges::to<string>();
    }

    static std::optional<integer> parse_integer(Context& ctx)
    {
        auto sv = extract_number_string_and_remove_underscore(ctx);
        using Caster = type_caster<floating, string, error_policy::optional>;

        if (sv.empty())
        {
            return std::nullopt;
        }

        int base = 10;

        if (sv.starts_with("0b") || sv.starts_with("0B"))
        {
            base = 2;
        }
    }

    static std::optional<floating> parse_float(Context& ctx)
    {
        auto s = extract_number_string_and_remove_underscore(ctx);
        std::basic_string_view<char> sv = s;
        using Caster = type_caster<floating, std::basic_string_view<char>, error_policy::optional>;

        if (sv.empty() || sv.front() == '.' || sv.back() == '.')
        {
            // Empty string, .7 or 7. is not permitted.
            return std::nullopt;
        }
        else if (sv == "nan" || sv == "+nan")
        {
            ctx.advance(sv.size());
            return std::numeric_limits<floating>::quiet_NaN();
        }
        else if (sv == "-nan")
        {
            ctx.advance(sv.size());
            return -std::numeric_limits<floating>::quiet_NaN();
        }
        else if (sv == "inf" || sv == "+inf")
        {
            ctx.advance(sv.size());
            return std::numeric_limits<floating>::infinity();
        }
        else if (sv == "-inf")
        {
            ctx.advance(sv.size());
            return -std::numeric_limits<floating>::infinity();
        }
        else
        {
            if (sv[0] == '+') 
            {
                sv.remove_prefix(1); // from_chars cannot parse '+'

                if (sv.size() == 1 || sv[1] == '-')
                {
                    // +-3.14
                    return std::nullopt;
                }
            }

            auto op = Caster()(sv);

            if (op) 
            {
                ctx.advance(sv.size());
            }
            return op;
        }
    }
};

template <typename Context>
struct toml_datatime_decoder;

template <typename Context>
struct toml_value_decoder;

template <typename Context>
struct toml_decoder
{
    using char_type = typename Context::value_type;

    static std::vector<string> decode_simple_keys(Context& ctx)
    {
        using StringDecoder = toml_string_decoder<Context>;
        
        std::vector<string> result;
        ctx.skip_whitespace();

        result.emplace_back(StringDecoder::decode_simple_key(ctx));

        while (1)
        {
            ctx.skip_whitespace();

            if (ctx.match('.', true))
            {
                result.emplace_back(StringDecoder::decode_simple_key(ctx));
                continue;
            }
            else
            {
                break;
            }
        }
        return result;
    }

    static std::vector<string> decode_std_section(Context& ctx)
    {
        if (!ctx.match('[', true))
        {
            throw toml_parse_error("Standard section must start with [");
        }

        ctx.skip_whitespace();
        auto result = decode_simple_keys(ctx);
        ctx.skip_whitespace();

        if (!ctx.match(']', true))
        {
            throw toml_parse_error("Standard section must end with ]");
        }
        return result;
    }

    static std::vector<string> decode_table_array_section(Context& ctx)
    {
        if (!ctx.match("[[", true))
        {
            throw toml_parse_error("Table array section must start with [[");
        }

        auto result = decode_simple_keys(ctx);
        ctx.skip_whitespace();

        if (!ctx.match("]]", true))
        {
            throw toml_parse_error("Table array section must end with ]]");
        }
        return result;
    }

    static std::vector<string> decode_section(Context& ctx)
    {
        if (ctx.match("[[", false))
        {
            return decode_table_array_section(ctx);
        }
        else if (ctx.match('[', false))
        {
            return decode_std_section(ctx);
        }
        else
        {
            throw toml_parse_error("Section must start with [ or [[");
        }
    }
};

}   // namespace detail

class decoder
{
    // Context
    parse_context m_ctx;  

    // Used for merging sections and entries
    collector m_coll;    

    static bool non_ascii(int ch)
    {
        return (0x80 <= ch && ch <= 0xD7FF)
            || (0xE000 <= ch && ch <= 0x10FFFF);
    }

public:

    decoder(std::string_view context) : m_ctx(context) { }

    value operator()()
    {
        parse();
        return m_coll.dispose();
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
            
            if (m_ctx.match(detail::comment_start_symbol))
            {
                parse_comment_optional();
                continue;
            }
            else if (m_ctx.match(detail::std_table_open))
            {
                parse_table();
            }
            else if (m_ctx.eof())
            {
                return;
            }
            else 
            {
                auto [keys, v] = parse_keyval();
                m_coll.add_entry(std::move(keys), std::move(v));
            }
            parse_comment_optional();
            m_ctx.skip_whitespace(); 
        }
    }

    void parse_table()
    {
        if (m_ctx.match(detail::array_table_open))
        {
            parse_array_table();
        }
        else
        {
            parse_std_table();
        }
    }

    void parse_array_table()
    {
        m_ctx.consume(detail::array_table_open);
        parse_wschar();
        m_coll.switch_to_array_table(parse_key());
        parse_wschar();
        check_and_throw(m_ctx.consume(detail::array_table_close), "Expected ]] after array table");
    }

    void parse_std_table()
    {
        m_ctx.consume(detail::std_table_open);
        parse_wschar();
        m_coll.switch_to_std_table(parse_key());
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
        // TODO:
        // TOML 1.1 supports newlines in inline tables and trailing commas.

        m_ctx.consume(detail::inline_table_open);
        parse_wschar();

        if (m_ctx.match(detail::inline_table_close))
        {
            m_ctx.advance_unchecked(1);
            return table(true);
        }
        else
        {
            table t(true);

            while (1)
            {
                auto [keys, v] = parse_keyval();
                try_put_value(std::move(keys), std::move(v), &t);
                parse_wschar();

                if (m_ctx.match(detail::inline_table_close))
                {
                    m_ctx.advance_unchecked(1); // eat '}'
                    return t;
                }
                else if (m_ctx.match(detail::inline_table_sep))
                {
                    m_ctx.advance_unchecked(1); // eat ','
                    parse_wschar();
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
        array retval(true);

        while (1)
        {
            skip_ws_comment_newline();
            check_and_throw(!m_ctx.eof(), "Expected ] after array.");

            if (m_ctx.consume(detail::array_close))
            {
                return retval;
            }
            retval.emplace_back(parse_val());
            skip_ws_comment_newline();
            check_and_throw(
                m_ctx.match(detail::array_sep) || m_ctx.match(detail::array_close), 
                "Expected , or ], but got {}", m_ctx.current());
            m_ctx.consume(detail::array_sep);
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
        // +-_.:eZTt [0-9|0x|0o|0b|inf|nan]
        constexpr std::string_view starts = "+-0123456789in";

        check_and_throw(
            starts.contains(m_ctx.current()),
            "Unknown character {} for value.", m_ctx.current()
        );

        constexpr std::string_view invalid = "=\r\n#,]}";
        size_t idx = 0;

        for (; idx < m_ctx.size() && !invalid.contains(m_ctx.peek(idx)); ++idx);

        auto sv = trim(m_ctx.slice(0, idx));
        m_ctx.advance_unchecked(idx);

        // We simply parse it as int, double and datetime separately.
        if (auto op = detail::parse_integer(sv); op)
        {
            return make_toml<integer>(op.value());
        }
        if (auto op = detail::parse_float(sv); op)
        {
            return make_toml<floating>(op.value());
        }
        if (auto op = detail::parse_datetime(sv); op)
        {
            // throw std::runtime_error("Not implemented.");
            return make_toml<datetime>(op.value());
        }
        throw_toml_parse_error("Error toml value.");
    }

    value parse_basic_string_or_multiline()
    {
        return m_ctx.match(detail::ml_basic_string_delim)
             ? parse_ml_basic_string()
             : parse_basic_string();
    }

    value parse_ml_basic_string()
    {
        m_ctx.consume(detail::ml_basic_string_delim); // eat """
        string retval;
        check_and_throw(!m_ctx.eof(), "Expected basic body.");

        if (m_ctx.current() == detail::newline)
        {
            m_ctx.advance_unchecked(1);   
        }

        // If a line end with '\\' after trim right, the linefeed 
        // will be ignored.
        while (!m_ctx.eof())
        {
            const auto ch = m_ctx.current();
            
            if (ch == detail::quotation_mark)
            {
                size_t count = 0;
                for (; m_ctx.peek(count) == detail::quotation_mark; ++count);
                check_and_throw(count < 6, "Too much quote.");

                if (count >= 3)
                {
                    retval.append(count - 3, detail::quotation_mark);
                    m_ctx.advance_unchecked(count - 3);
                    m_ctx.consume(detail::ml_basic_string_delim);
                    return retval;
                }
                else
                {
                    retval.append(count, detail::quotation_mark);
                    m_ctx.advance_unchecked(count);
                }
            }
            else if (ch == '\\')
            {
                m_ctx.advance_unchecked(1);
                check_and_throw(!m_ctx.eof(), "Error EOF multiline basic string.");

                // If a line end with '/' after trimming.
                if (trim(m_ctx.read_line()).empty())
                {   
                    m_ctx.skip_whitespace();
                }
                else
                {
                    switch (*m_ctx)
                    {
                        case 'b': retval += '\b'; ++m_ctx; break;
                        case 't': retval += '\t'; ++m_ctx; break;
                        case 'n': retval += '\n'; ++m_ctx; break;
                        case 'f': retval += '\f'; ++m_ctx; break;
                        case 'r': retval += '\r'; ++m_ctx; break;
                        case '"': retval += '"'; ++m_ctx; break;
                        case '\\': retval += '\\'; ++m_ctx; break;
                        case 'u': detail::decode_unicode<4>(std::back_inserter(retval), ++m_ctx); break;
                        case 'U': detail::decode_unicode<8>(std::back_inserter(retval), ++m_ctx); break;
                        default: throw_toml_parse_error("Illegal character {} after \\", m_ctx.current());
                    }
                }

            }
            else
            {
                retval += ch;
                m_ctx.advance_unchecked(1);
            }
        }
        throw_toml_parse_error("Expected \"\"\" after multi-line basic string.");
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

        while (!m_ctx.eof())
        {
            const auto ch = m_ctx.current();
            
            if (ch == detail::apostrophe)
            {
                size_t count = 0;
                for (; m_ctx.peek(count) == detail::apostrophe; ++count);
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
            else
            {
                // Multi-line literal strings are surrounded by three single 
                // quotes on each side and allow newlines. Like literal strings, 
                // there is no escaping whatsoever. A newline immediately 
                // following the opening delimiter will be trimmed. All other 
                // content between the delimiters is interpreted as-is without modification.
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
        else if (m_ctx.match(detail::quotation_mark))
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
            // return true;
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
        // m_ctx.consume(detail::quotation_mark);
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

        if (m_ctx.eof())
        {
            return;
        }

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
        m_ctx.advance(1);
    }
};

inline constexpr struct
{
    static value operator()(std::string source)
    {
        return decoder(source)();
    }
} loads;

inline constexpr struct 
{
    static value operator()(const char* filename)
    {
        // Windows
        constexpr const char* linefeed = "\r\n";
        return loads(cpp::read_file_context(filename, linefeed));
    }

    static value operator()(const std::string& filename)
    {
        return operator()(filename.c_str());
    }
} load;

// inline value loads(std::string source)
// {
//     return decoder(source)();
// }

// inline value load(const char* filename)
// {
//     // Windows
//     constexpr const char* linefeed = "\r\n";
//     return loads(cpp::read_file_context(filename, linefeed));
// }

} // namespace cpp::config::toml

namespace cpp::config
{

// template <>
// struct value_parser<toml::value>
// {
//     static toml::value operator()(std::string source)
//     {
//         return toml::decoder(source).parse_val();
//     }
// };

}

