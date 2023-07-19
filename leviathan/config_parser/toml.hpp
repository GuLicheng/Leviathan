/*
    Reference:
        - https://mojotv.cn/2018/12/26/what-is-toml
        - https://toml.io/cn/v1.0.0
        - https://github.com/toml-lang/toml/blob/1.0.0/toml.abnf
        - https://github.com/BurntSushi/toml-test
*/

#pragma once

#include "common.hpp"
#include "toml_value.hpp"

// #include <leviathan/string/string_extend.hpp>

#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <string_view>
#include <variant>
#include <ranges>
#include <chrono>
#include <limits>
#include <format>

namespace leviathan::config::toml
{
    using std::string_view;
    using std::string;
    
    namespace detail
    {
        struct config
        {
            constexpr static int value = 256;

            int operator()(size_t x) const
            {
                [[assume(x < value)]];
                static string_view sv = R"(_-. )";
                return isalnum(x) || sv.contains(x);
            }
        };

        /**
         * @brief Split key by '.'.
         * 
         * @return Result of split. The result will not be empty.
        */
        inline std::vector<string_view> split_keys(string_view key)
        {
            key = ltrim(key);

            // assert(key.size());
            if (key.empty())
            {
                throw_toml_parse_error("The input key is empty");
            }

            std::vector<string_view> result;

            static auto table = make_character_table(config());

            auto parse_quote = [&](char ch) {

                auto sv = key;

                assert(sv.size() && sv.front() == ch);

                sv.remove_prefix(1);

                auto next_quote = sv.find(ch);

                if (next_quote == sv.npos)
                {
                    throw_toml_parse_error("Parsing quote key error.");
                }

                result.emplace_back(sv.substr(0, next_quote));

                sv.remove_prefix(1 + next_quote);

                key = ltrim(sv);
            };

            while (1)
            {
                if (key.empty())
                {
                    throw_toml_parse_error("Invalid keys: {}", key);
                }

                char ch = key.front();

                if (ch == '"')
                {
                    parse_quote('"');
                }
                else if (ch == '\'')
                {
                    parse_quote('\'');
                }
                else if (table[ch])
                {
                    // pase bar string
                    auto head = key.begin(), tail = key.end();
                    for (; head != tail; ++head)
                    {
                        auto ch2 = *head;
                        if (ch2 == '.' || ch2 == ' ')
                        {
                            // result.emplace_back(key.begin(), head);
                            // key = ltrim(string_view(head, tail));
                            break;
                        }
                        else if (!table[*head])
                        {
                            throw_toml_parse_error("Invalid character when parsing bare key."); 
                        }
                    }
                    result.emplace_back(key.begin(), head);
                    key = ltrim(string_view(head, tail));
                }
                else
                {
                    throw_toml_parse_error("Invalid character when parsing bare key.");
                }

                if (key.empty())
                {
                    return result;
                }
            
                ch = key.front();

                if (ch == '.')
                {
                    key.remove_prefix(1);
                    key = ltrim(key);
                    continue;
                }
                else
                {
                    throw_toml_parse_error("Expected '=' or '.', but got {}", ch);
                }
            }
        }
    }

    class parser
    {
        string m_context;                                   // Save origin string.
        std::vector<std::string_view> m_lines;              // Split origin string line by line.
        std::vector<std::string_view>::iterator m_lineit;   // Current line.
        string_view m_line;                                 // Current line for parsing. For simplicity, we use string_view to replace iterator/pointer. 
        toml_table m_global;                                // Toml root table.
        toml_table* m_cur_table;                            // Current table root(section).

    public:

        parser(string context) 
            : m_context(std::move(context)) { }
    
        toml_value operator()() &&
        {
            read_lines();
            for (auto line : m_lines)
            {
                // std::cout << line << '\n';
                // std::cout << std::format("line = ({}) and last char is ({})\n", line, line.back());
            }
            m_lineit = m_lines.begin();
            m_cur_table = &m_global;
            parse();
            return toml_value(std::move(m_global));
        }

    private:

        void parse()
        {
            while (getline())
            {
                m_line = ltrim(m_line);

                // Empty line or just commit.
                if (m_line.empty() || m_line.front() == '#')
                {   
                    // U+0000 - U+0008 and U+000A - U+001F,U+007F should not appear in commit.
                    continue;
                }

                if (m_line.front() == '[')
                {
                    // [table] or [[table_array]]
                    parse_table_or_table_array();
                }
                else
                {
                    // parse key and value.
                    parse_entry();
                }
            }
        }

        void parse_entry()
        {
            skip_whitespace();

            assert(m_line.size());

            if (m_line.front() == '=')
            {
                throw_toml_parse_error("Empty Key.");
            }

            // parse key
            assert(m_line.size());

            auto idx = m_line.find('='); 
            
            if (idx == m_line.npos)
            {
                throw_toml_parse_error("Expected '=' when parsing entry");
            }

            auto keys = detail::split_keys(m_line.substr(0, idx));

            advance_unchecked(idx); // eat key.

            skip_whitespace();

            // match '='
            if (!match_and_advance('='))
            {
                throw_toml_parse_error("Expected '=' after key.");
            }

            skip_whitespace();

            // parse value
            auto value = parse_value();

            try_put_value(keys, std::move(value));

            consume_whitespace_and_comment();
        }

        void try_put_value(const std::vector<string_view>& keys, toml_value&& val)
        {
            bool ok = false;

            auto t = m_cur_table;

            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                auto [it, succeed] = t->try_emplace(toml_string(keys[i]), toml_table());

                if (!succeed && !it->second.is<toml_table>())
                {
                    throw_toml_parse_error("Key conflict");
                }

                ok = ok || succeed;
                t = it->second.as_ptr<toml_table>();
            }

            if (!t->try_emplace(toml_string(keys.back()), std::move(val)).second)
            {
                throw_toml_parse_error("Value already exits");
            }
        }

        void parse_table_or_table_array()
        {
            assert(m_line.front() == '[');

            advance_unchecked(1); // eat [

            if (m_line.empty())
            {
                throw_toml_parse_error("Unexpected end of table.");
            }

            if (current() == '[')
            {
                parse_table_array();
            }
            else
            {
                parse_table();
            }
        }

        void parse_table_array()
        {
            advance_unchecked(1); // eat '['.

            const auto idx = m_line.find(']');

            if (idx == m_line.npos)
            {
                throw_toml_parse_error("Unexpected end of table array");
            }

            auto table_name = m_line.substr(0, idx);

            try_create_table_array(table_name);

            m_line.remove_prefix(idx + 1); // remove table name and ']'.

            if (!match_and_advance(']'))
            {
                throw_toml_parse_error("Unexpected end of table array");
            }

            consume_whitespace_and_comment();
        }

        void parse_table()
        {
            const auto idx = m_line.find(']');

            if (idx == m_line.npos)
            {
                throw_toml_parse_error("Unexpected end of table.");
            }

            auto table_name = m_line.substr(0, idx);

            try_create_single_table(table_name);

            m_line.remove_prefix(idx + 1); // remove table name and ']'.

            consume_whitespace_and_comment();
        }

        void try_create_table_array(string_view table_name)
        {
            auto keys = detail::split_keys(table_name);

            toml_table* root = &m_global;

            for (auto key : keys | std::views::take(keys.size() - 1)) // drop last
            {
                auto [it, succeed] = root->try_emplace(toml_string(key), toml_table());

                if (!succeed && !it->second.is<toml_table>())
                {
                    throw_toml_parse_error("Table array conflict.");
                }

                root = it->second.as_ptr<toml_table>();
            }

            auto [it, succeed] = root->try_emplace(toml_string(keys.back()), toml_array(false));

            if (!succeed)
            {
                if (!it->second.is<toml_array>())
                {
                    throw_toml_parse_error("Table array conflict.");
                }
                if (it->second.as_ptr<toml_array>()->is_array())
                {
                    throw_toml_parse_error("Cannot change the static array.");
                }
            }

            m_cur_table = it->second.as_ptr<toml_array>()->emplace_back(toml_table()).as_ptr<toml_table>();
        }

        void try_create_single_table(string_view table_name)
        {
            auto keys = detail::split_keys(table_name);

            toml_table* root = &m_global;

            bool ok = false;   // Check whether all keys are already exist.

            for (auto key : keys)
            {
                assert(root);
                auto [it, succeed] = root->try_emplace(toml_string(key), toml_table());
                
                if (!succeed && !it->second.is<toml_table>())
                {
                    throw_toml_parse_error("Table conflict.");
                }

                ok = ok || succeed;
                root = it->second.as_ptr<toml_table>();
            }

            if (!ok)
            {
                throw_toml_parse_error("Redefinition table.");
            }

            m_cur_table = root;
        }

        /* -------------------------------- Functions for parsing value -------------------------------- */

        toml_value parse_value()
        {
            skip_whitespace();

            if (m_line.empty())
            {
                throw_toml_parse_error("Value error");
            }

            switch (current())
            {
                case '[': return parse_array();
                case '{': return parse_inline_table();
                case 't': return parse_true();
                case 'f': return parse_false();
                case '"': return parse_basic_string();
                case '\'': return parse_literal_string();
                default: return parse_number_or_data_time();
            }
        }

        toml_value parse_number_or_data_time()
        {
            static string_view valid_first_char = "+-0123456789in";

            if (!valid_first_char.contains(current()))
            {
                throw_toml_parse_error("Unknown characters");
            }

            enum struct value_kind
            {
                unknown,
                error, 
                inf,
                nan,
                number,
                floating_or_data_time,
                floating,
                data_time,
            };

            struct value_character_config
            {
                using type = value_kind;

                value_kind operator()(size_t x) const
                {
                    if (isdigit(x))
                    {
                        return value_kind::unknown;
                    }

                    switch (x)
                    {
                        case 'i': return value_kind::inf;
                        case 'n': return value_kind::nan;
                        case '+': 
                        case '_': 
                        case '-': return value_kind::number;
                        case 'e':
                        case 'E': return value_kind::floating;
                        case ' ':
                        case 'T':
                        case 'Z': return value_kind::data_time;
                        case '.': return value_kind::floating_or_data_time;
                        default: return value_kind::error;
                    }
                }
            };

            static string_view sv = "=\n,]}";

            auto startptr = m_line.data();

            for (; m_line.size() && !sv.contains(current()); advance_unchecked(1));

            auto endptr = m_line.data();

            auto context = rtrim(string_view(startptr, endptr));

            auto contains_characters = [](string_view c, const char* strs) {
                return c.find_first_of(strs) != c.npos;
            };

            auto ch = context.front();

            if (ch == '+' || ch == '-') 
            {
                // Only integer and floating can started with "+-".
                // Integer may contains '_', so we must remove the '_' first.
            }
            else if (ch == 'i' || ch == 'n') 
            {
                // i -> inf, n -> nan.
                auto value = from_chars_to_optional<double>(context.begin(), context.end());
                
                if (!value)
                {
                    throw_toml_parse_error("Error floating number.");
                }
                return toml_float(*value);
            }
            else
            {
                // Try parse as integer first, then fall back to double, and last to data time.   
            }
        }

        toml_value parse_true()
        {
            if (!compare_literal_and_advance("true"))
            {
                throw_toml_parse_error("Parse true error.");
            }
            return toml_boolean(true);
        }

        toml_value parse_false()
        {
            if (!compare_literal_and_advance("false"))
            {
                throw_toml_parse_error("Parse false error.");
            }
            return toml_boolean(false);
        }

        toml_value parse_basic_string()
        {
            return m_line.compare(R"(""")") == 0 
                 ? parse_multi_line_basic_string()
                 : parse_single_line_basic_string();
        }

        toml_value parse_literal_string()
        {
            return m_line.compare(R"(''')") == 0 
                 ? parse_multi_line_literal_string()
                 : parse_single_line_literal_string();
        }

        toml_value parse_multi_line_basic_string()
        {
            advance_unchecked(3); // eat """
            throw_toml_parse_error("Not implement.");
        }

        toml_value parse_single_line_basic_string()
        {
            advance_unchecked(1); // eat "

            if (m_line.empty())
            {
                throw toml_parse_error("single line string cannot just with \"");
            }

            toml_string s;

            while (1)
            {
                char ch = current();

                if (ch == '"')
                {
                    advance_unchecked(1); // eat "
                    return toml_value(std::move(s));
                }

                if (ch == '\\')
                {
                    advance_unchecked(1); // eat '\'

                    if (m_line.empty())
                    {
                        throw_toml_parse_error("Expected character after \\");
                    }

                    auto decode_unicode = [&, this]<size_t N>(string_view sv) {
                        if (sv.size() != N)
                        {
                            throw_toml_parse_error("Character is not enough");
                        }
                        auto codepoint = decode_unicode_from_char<N>(sv.data());
                        encode_unicode_to_utf8(std::back_inserter(s), codepoint);
                        advance_unchecked(N);
                    };

                    switch (current())
                    {
                        case '"': s += '"'; break;    // quote
                        case '\\': s += '\\'; break;  // reverse solidus
                        case '/': s += '/'; break;    // solidus
                        case 'b': s += '\b'; break;   // backspace
                        case 'f': s += '\f'; break;   // formfeed
                        case 'n': s += '\n'; break;   // linefeed
                        case 'r': s += '\r'; break;   // carriage return
                        case 't': s += '\t'; break;   // horizontal tab
                        case 'u': decode_unicode.template operator()<4>(m_line.substr(1, 4)); break;
                        case 'U': decode_unicode.template operator()<8>(m_line.substr(1, 8)); break;
                        default: throw_toml_parse_error("Illegal character after \\");
                    }
                }
                else
                {
                    s += ch;
                }

                advance_unchecked(1);
            }
        }

        toml_value parse_multi_line_literal_string()
        {
            throw_toml_parse_error("Not implement.");
        }

        toml_value parse_single_line_literal_string()
        {
            advance_unchecked(1);

            auto idx = m_line.find('\'');

            if (idx == m_line.npos)
            {
                throw_toml_parse_error("Unexpected end of single literal string");
            }

            toml_string s = toml_string(m_line.substr(0, idx));

            advance_unchecked(idx + 1);  // eat str and '\''

            return toml_value(std::move(s));
        }

        toml_value parse_array()
        {
            assert(m_line.front() == '[');

            advance_unchecked(1); // eat '['

            skip_whitespace_and_empty_line();

            if (m_line.empty())
            {
                throw_toml_parse_error("Unexpected end of array.");
            }
            else
            {
                toml_array arr(true);

                while (1)
                {
                    if (current() == ']')
                    {
                        advance_unchecked(1); // eat ']'
                        return toml_value(std::move(arr));
                    }

                    auto value = parse_value();

                    arr.emplace_back(std::move(value));

                    skip_whitespace_and_empty_line();

                    if (m_line.empty())
                    {
                        throw_toml_parse_error("Unexpected end of array.");
                    }

                    if (current() == ']')
                    {
                        advance_unchecked(1);

                        if (!arr.is_all_same_type())
                        {   
                            throw_toml_parse_error("Elements in array must share same type.");
                        }
                        return toml_value(std::move(arr));
                    }

                    if (current() == ',')
                    {
                        advance_unchecked(1); // eat ,
                        skip_whitespace_and_empty_line();
                    }
                    else
                    {
                        throw_toml_parse_error("Expected , or ] after value.");
                    }
                }
            }
        }

        toml_value parse_inline_table()
        {
            throw_toml_parse_error("Not Implement.");
        }

        /* -------------------------------- Functions for reader -------------------------------- */
        
        /**
         * @brief Split context by line and remove empty line.
        */
        void read_lines()
        {
            // Every line may ends with \r\n on Windows.
            // We replace all \r\n to \n, this is not efficient but simple.
            m_context = replace(std::move(m_context), "\r\n", "\n");

            // Directly set 
            // std::ranges::split_view<std::ranges::ref_view<std::string>, std::string_view>
            // as a field may be better.
            for (auto line : m_context | std::views::split('\n'))
            {
                m_lines.emplace_back(line);
            }
        }

        bool getline()
        {
            if (m_lineit == m_lines.end())
            {
                m_line = "";
                return false;
            }
            m_line = *m_lineit++;
            return true;
        }

        bool compare_literal_and_advance(string_view literal)
        {
            // compare, string_view::substr will check length automatically.
            if (m_line.compare(0, literal.size(), literal) != 0)
            {
                return false;
            }
            advance_unchecked(literal.size());
            return true;
        }

        /**
         * @brief Remove the blank and commit of current line.
         *  After removing, the m_line should be empty. This
         *  function is just for handing error case, and ignore 
         *  it will not cause parse error.
        */
        void consume_whitespace_and_comment()
        {
            m_line = ltrim(m_line);
            if (m_line.size() && m_line.front() != '#')
            {
                throw_toml_parse_error("Unknown character.");
            }
            m_line = "";
        }

        bool match_and_advance(char ch)
        {
            if (m_line.empty())
            {
                return false;
            }
            bool result = current() == ch;
            advance_unchecked(1);
            return result;
        }

        void skip_whitespace()
        {
            struct whitespace_config
            {
                constexpr int operator()(size_t i) const
                {
                    [[assume(i < 128)]];
                    constexpr std::string_view sv = " \r\n\t";
                    return sv.contains(i);
                }
            };

            static auto whitespaces = make_character_table(whitespace_config());

            auto is_whitespace = [](char ch) { return whitespaces[ch]; };
            for (; m_line.size() && is_whitespace(current()); advance_unchecked(1));
        }

        /**
         * @brief This function will try to skip newline and comment.
        */
        void skip_whitespace_and_empty_line()
        {
            while (1)
            {
                skip_whitespace();
                if (m_line.size())
                {
                    if (current() == '#')
                    {
                        getline();
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                if (!getline())
                {
                    return;
                }
            }
        }

        char peek(int n) const
        { return m_line[n]; }

        char current() const
        { return peek(0); }

        void advance_unchecked(size_t n)
        { m_line.remove_prefix(n); }
    };

    toml_value load(string context)
    { return parser(std::move(context))(); }

    toml_value parse_toml(const char* filename)
    { return load(read_file_contents(filename)); }

} // namespace leviathan::config::toml

namespace leviathan::toml
{
    using namespace ::leviathan::config::toml;
}

