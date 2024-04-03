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
#include <cmath>
#include <unordered_map>
#include <vector>
#include <regex>
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
    
        /**
         * @brief This class offer some utils helper functions.
        */
        struct parser_helper
        {
            static optional<string> remove_underscore(string_view sv)
            {
                string s;

                if (sv.front() == '_' || sv.back() == '_')
                {
                    // throw_toml_parse_error("underscore cannot appear at begin or end.");
                    return nullopt;
                }

                struct underscore_config
                {
                    int operator()(size_t x) const
                    {
                        [[assume(x < 256)]];
                        static string_view sv = "0123456789abcdefABCDEF";
                        return sv.contains(x);
                    }
                };

                static auto table = make_character_table(underscore_config());

                for (size_t i = 0; i < sv.size(); ++i)
                {
                    auto ch = sv[i];
                    if (ch == '_')
                    {
                        if (!table[sv[i - 1]] || !table[sv[i + 1]])
                        {
                            // throw_toml_parse_error("Underscore should between two number.");
                            return nullopt;
                        }
                    }
                    else
                    {
                        s += ch;
                    }
                }
                return s;
            }

            static optional<toml_value> parse_toml_integer(string_view sv)
            {
                auto s = remove_underscore(sv);

                if (!s)
                {
                    return nullopt;
                }

                sv = *s;  // Replace sv with s which is removed underscore.

                if (sv.front() == '+') // from_chars cannot parse '+'
                {
                    sv.remove_prefix(1);
                }

                int base = 10;

                if (sv.size() > 1 && sv.front() == '0')
                {
                    switch (sv[1])
                    {
                        case 'x':
                        case 'X': base = 16; break;
                        case 'o':
                        case 'O': base = 8; break;
                        case 'b':
                        case 'B': base = 2; break;
                        default: return nullopt;  
                    }
                    sv.remove_prefix(2);
                }

                if (auto op = from_chars_to_optional<toml_integer>(sv, base); op)
                {
                    return *op;
                }
                return nullopt;
            }

            static optional<toml_value> parse_toml_float(string_view sv)
            {
                auto s = remove_underscore(sv);

                if (!s)
                {
                    return nullopt;
                }

                sv = *s;  // Replace sv with s which is removed underscore.

                // .7 or 7. is not permitted.
                if (sv.front() == '.' || sv.back() == '.')
                {
                    return nullopt;
                }

                toml_float sign = [ch = sv.front()]() {
                    switch (ch) 
                    {
                        case '+': return 1;
                        case '-': return -1;
                        default: return 0;
                    }
                }();

                if (sign == 0)
                {
                    // "nan" and "inf" is valid for std::from_chars
                    if (auto op = from_chars_to_optional<toml_float>(sv); op) 
                    {
                        return *op;
                    }
                    return nullopt;
                }

                sv.remove_prefix(1); // eat '+' or '-'

                if (sv.front() == '.')
                {
                    // +.7 is also not permitted.
                    return nullopt;
                }

                if (auto op = from_chars_to_optional<toml_float>(sv); op) 
                {
                    // std::copysign is valid for nan/inf
                    return std::copysign(*op, sign);
                }
                return nullopt;
            }

            static optional<toml_datetime> parse_toml_datetime(string_view sv)
            {
                static std::regex re(R"(^(?:(\d+)-(0[1-9]|1[012])-(0[1-9]|[12]\d|3[01]))?([\sTt])?(?:([01]\d|2[0-3]):([0-5]\d):([0-5]\d|60)(\.\d+)?((?:[Zz])|(?:[\+|\-](?:[01]\d|2[0-3])(?::[0-6][0-9])?(?::[0-6][0-9])?))?)?$)");

                std::string s(sv);
                
                std::smatch pieces_match;

                if (std::regex_match(s , pieces_match, re))
                {
                    toml_datetime dt;

                    // auto to_int = [](auto& slice) {
                    //     return from_chars_to_optional<int>(slice.str()).value();
                    // };

                    // dt.year = to_int(pieces_match[2]);
                    // dt.month = to_int(pieces_match[3]);
                    // dt.day = to_int(pieces_match[4]);

                    // dt.hour = to_int(pieces_match[6]);
                    // dt.minute = to_int(pieces_match[7]);
                    // dt.second = to_int(pieces_match[8]);

                    // auto microsecond = to_int(pieces_match[9]);

                    // if (pieces_match[10].str().size() && pieces_match[10] != 'Z')
                    // {

                    // }
                    return dt;
                }
                else
                {
                    return nullopt;
                }
            }
        };
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

        void try_put_value(const std::vector<string_view>& keys, toml_value&& val, toml_table* table = nullptr)
        {
            bool ok = false;

            auto t = table ? table : m_cur_table;

            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                auto [it, succeed] = t->try_emplace(toml_string(keys[i]), toml_table());

                if (!succeed)
                {
                    if (!it->second.is<toml_table>())
                    {
                        throw_toml_parse_error("Key conflict");
                    }
                    if (it->second.as_ptr<toml_table>()->is_inline_table())
                    {
                        throw_toml_parse_error("Inline table cannot be modified.");
                    }
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

            m_cur_table = it->second.as_ptr<toml_array>()->emplace_back(toml_table(true)).as_ptr<toml_table>();
            // The last sub-table is defined so that the table with same name is not allowed.
            m_cur_table->define_table();
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
                    // already exist and the sub value is not a table
                    throw_toml_parse_error("Table conflict.");
                }

                ok = ok || succeed;
                root = it->second.as_ptr<toml_table>();
            }

            if (!ok && root->is_defined())
            {
                throw_toml_parse_error("Redefinition table.");
            }

            m_cur_table = root;
            m_cur_table->define_table();
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
                default: return parse_number_or_date_time();
            }
        }

        toml_value parse_number_or_date_time()
        {
            static string_view sv = " \n,]}";

            auto idx = m_line.find_first_of(sv);

            auto context = m_line.substr(0, idx);

            // Try parse as integer first
            if (auto op = detail::parser_helper::parse_toml_integer(context); op)
            {
                advance_unchecked(context.size());
                return std::move(*op);
            }

            // Then fallback to floating.
            if (auto op = detail::parser_helper::parse_toml_float(context); op)
            {
                advance_unchecked(context.size());
                return std::move(*op);
            }

            if (auto op = detail::parser_helper::parse_toml_datetime(context); op)
            {
                advance_unchecked(context.size());
                return std::move(*op);
            }

            throw_toml_parse_error("Invalid toml value.");
        }

        toml_value parse_date_time(string_view context)
        {
            throw_toml_parse_error("Not implemented");
        }

        toml_value parse_inf()
        {
            if (!compare_literal_and_advance("inf"))
            {
                throw_toml_parse_error("Parse inf error.");
            }
            return std::numeric_limits<double>::infinity();
        }

        toml_value parse_nan()
        {
            if (!compare_literal_and_advance("inf"))
            {
                throw_toml_parse_error("Parse inf error.");
            }
            return std::numeric_limits<double>::quiet_NaN();
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
            return m_line.starts_with(R"(""")") 
                 ? parse_multi_line_basic_string()
                 : parse_single_line_basic_string();
        }

        toml_value parse_literal_string()
        {
            return m_line.starts_with(R"(''')") 
                 ? parse_multi_line_literal_string()
                 : parse_single_line_literal_string();
        }

        toml_value parse_multi_line_basic_string()
        {
            advance_unchecked(3); // eat """
            toml_string context;

            if (m_line.size() == 1)
            {
                if (current() == '\n')
                {
                    advance_unchecked(1);
                }
                else if (current() == '\\')
                {
                    advance_unchecked(1);
                    skip_whitespace_and_empty_line();
                }
            }

            while (1)
            {
                while (m_line.empty())
                {
                    if (!getline())
                    {
                        throw_toml_parse_error("Unexpected end of multi line basic string.");
                    }
                    if (context.size())
                    {
                        context += '\n';
                    }
                }

                if (current() != '"')
                {
                    if (all_blank())
                    {
                        advance_unchecked(1);
                        skip_whitespace_and_empty_line();
                        continue;
                    }
                    context += current();
                    advance_unchecked(1);
                }
                else if (current() == '\\')
                {
                    advance_unchecked(1); // eat '\'

                    switch (current())
                    {
                        case 'b': context += '\b'; break;
                        case 't': context += '\t'; break;
                        case 'n': context += '\n'; break;
                        case 'f': context += '\f'; break;
                        case 'r': context += '\r'; break;
                        case '"': context += '"'; break;
                        case '\\': context += '\\'; break;
                        case 'u': decode_unicode<4>(context, m_line.substr(1, 4)); break;
                        case 'U': decode_unicode<8>(context, m_line.substr(1, 8)); break;
                        default: throw_toml_parse_error("Illegal character {} after \\", current());
                    }
                    advance_unchecked(1);
                }
                else
                {
                    if (m_line.compare(R"("""""")") == 0)
                    {
                        throw_toml_parse_error("Too much quote.");
                    }

                    if (m_line.compare(R"(""")") == 0)
                    {
                        advance_unchecked(3);
                        while (m_line.size() && current() == '"')
                        {
                            context += '"';
                        }
                        break;
                    }
                    else
                    {
                        context += '"';
                        advance_unchecked(1);
                    }
                }
            }
            return context;
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
                        case 'u': decode_unicode<4>(s, m_line.substr(1, 4)); break;
                        case 'U': decode_unicode<8>(s, m_line.substr(1, 8)); break;
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
            advance_unchecked(3); // eat '''
            toml_string context;

            if (m_line.size() == 1)
            {
                if (current() == '\n')
                {
                    advance_unchecked(1);
                }
                else if (current() == '\\')
                {
                    advance_unchecked(1);
                    skip_whitespace_and_empty_line();
                }
            }

            while (1) 
            {
                while (m_line.empty())
                {
                    if (!getline())
                    {
                        throw_toml_parse_error("Unexpected multi-line basic string.");
                    }
                    if (context.size())
                    {
                        context += '\n';
                    }
                }

                if (current() != '\'')
                {
                    if (all_blank())
                    {
                        advance_unchecked(1);
                        skip_whitespace_and_empty_line();
                        continue;
                    }
                    context += current();
                    advance_unchecked(1);
                }
                else
                {
                    // Sequences of three or more single quotes are not permitted.
                    if (m_line.compare("''''''") == 0)
                    {
                        throw_toml_parse_error("Too much quote.");
                    }

                    if (m_line.compare("'''") == 0)
                    {
                        advance_unchecked(3);
                        while (m_line.size() && current() == '\'')
                        {
                            context += '\'';
                        }
                        break;
                    }
                    else
                    {
                        context += '\'';
                        advance_unchecked(1);
                    }
                }
            }

            return context;
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

                    // toml 1.0 allow mixed-type
                    // if (arr.size() && value.index() != arr.front().index())
                    // {
                    //     throw_toml_parse_error("Elements in array must share same type.");
                    // }

                    arr.emplace_back(std::move(value));

                    skip_whitespace_and_empty_line();

                    if (m_line.empty())
                    {
                        throw_toml_parse_error("Unexpected end of array.");
                    }
                    if (current() == ']')
                    {
                        advance_unchecked(1);
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
            advance_unchecked(1); // eat '{'.

            skip_whitespace();

            if (m_line.empty())
            {
                throw_toml_parse_error("Unexpected end of inline table.");
            }

            toml_table table(true);
            table.define_table();

            if (current() == '}')
            {
                advance_unchecked(1);
                return toml_value(std::move(table));
            }
            else
            {
                while (1)
                {
                    if (m_line.front() == '=')
                    {
                        throw_toml_parse_error("Empty Key.");
                    }

                    // parse key
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

                    // m_cur_table = &table;
                    try_put_value(keys, std::move(value), &table);

                    skip_whitespace();

                    if (m_line.empty())
                    {
                        throw_toml_parse_error("Unexpected end of inline table.");
                    }
                    if (current() == '}')
                    {
                        advance_unchecked(1);
                        return toml_value(std::move(table));
                    }
                    if (current() == ',')
                    {
                        advance_unchecked(1);
                        skip_whitespace();
                    }
                    else
                    {
                        throw_toml_parse_error("Expected , or } after value.");
                    }
                }

            }
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
                    [[assume(i < 256)]];
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

        void try_skip_whitespace_and_empty_line()
        {
            assert(current() == '\\');
            advance_unchecked(1);
            if (m_line.size() == 1 && current() == '\n')
            {
                skip_whitespace_and_empty_line();
            }
        }

        char peek(int n) const
        { return m_line[n]; }

        char current() const
        { return peek(0); }

        void advance_unchecked(size_t n)
        { m_line.remove_prefix(n); }

        bool all_blank() const
        {
            return m_line.size() 
                && current() == '\\' 
                && m_line.substr(1).find_first_not_of(" \t") == m_line.npos;
        }

        template <size_t N>
        void decode_unicode(toml_string& context, string_view sv) 
        {
            if (sv.size() != N)
            {
                throw_toml_parse_error("Too small characters for unicode");
            }
            auto codepoint = decode_unicode_from_char<N>(sv.data());
            encode_unicode_to_utf8(std::back_inserter(context), codepoint);
            advance_unchecked(N);
        }
    };

    inline toml_value load(string context)
    { return parser(std::move(context))(); }

    inline toml_value parse_toml(const char* filename)
    { return load(read_file_contents(filename)); }

} // namespace leviathan::config::toml

namespace leviathan::toml
{
    using namespace ::leviathan::config::toml;
}

#include <format>
#include "convert.hpp"

template <typename CharT>
struct std::formatter<leviathan::toml::toml_value, CharT>
{
    constexpr auto parse(auto& ctx) 
    { return ctx.begin(); }

    auto format(const auto& x, auto& ctx) const
    {
        auto jv = leviathan::config::toml2json(x);
        return std::formatter<leviathan::json::json_value, CharT>().format(jv, ctx);
    }
};