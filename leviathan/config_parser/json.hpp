// https://www.json.org/json-en.html
#pragma once

#include "json_value.hpp"
#include "common.hpp"

#include <sstream>
#include <utility>
#include <string>
#include <iostream>
#include <compare>
#include <unordered_map>
#include <algorithm>
#include <type_traits>
#include <format>
#include <assert.h>

namespace leviathan::config::json
{
    using std::string_view;
    using std::string;

    class parser
    {
        string m_context;
        string_view m_cur;

    public:

        parser(string context) 
            : m_context(std::move(context)), m_cur(m_context) { }
    
        json_value operator()() &&
        {
            // "A JSON payload should be an object or array."
            // return parse_array_or_object();
            auto root = parse_value();

            if (!root)
            {
                return root;
            }

            skip_whitespace();

            if (m_cur.empty())
            {
                return root;
            }

            return return_with_error_code(error_code::multi_value);
        }

    private:

        json_value parse_array_or_object()
        {
            skip_whitespace();

            switch (current())
            {
                case '{': return parse_object();
                case '[': return parse_array();
                default: return return_with_error_code(error_code::error_payload);
            }
        }

        json_value parse_value()
        {
            skip_whitespace(); // necessary?

            if (m_cur.empty())
            {
                return return_with_error_code(error_code::eof_error);
            }

            switch (current())
            {
                case 't': return parse_true();
                case 'n': return parse_null();
                case 'f': return parse_false();
                case '[': return parse_array();
                case '{': return parse_object();
                case '"': return parse_string();
                default: return parse_number();
            }
        }

        json_value parse_array()
        {
            advance_unchecked(1); // eat '['

            skip_whitespace();

            json_array arr;

            if (current() == ']')
            {
                advance_unchecked(1); // eat ']'
                return json_value(std::move(arr));
            }   
            else 
            {
                while (1)
                {
                    auto value = parse_value();

                    if (!value)
                    {
                        // return return_with_error_code(error_code::illegal_array);
                        return value; 
                    }

                    arr.emplace_back(std::move(value));

                    skip_whitespace();

                    if (current() == ']')
                    {
                        advance_unchecked(1); // eat ']'
                        return json_value(std::move(arr));
                    }

                    if (!match_and_advance(','))
                    {
                        return return_with_error_code(error_code::illegal_array);
                    }

                    skip_whitespace();
                }
            }
        }

        json_value parse_true()
        {
            return compare_literal_and_advance("true") 
                ? json_boolean(true) 
                : return_with_error_code(error_code::illegal_literal); 

            // Advance and check rest characters may faster.
            // advance_unchecked(1); return compare_literal_and_advance("rue") ...
        }
        
        json_value parse_false()
        {
            return compare_literal_and_advance("false") 
                ? json_boolean(false) 
                : return_with_error_code(error_code::illegal_literal); 
        }

        json_value parse_null()
        {
            return compare_literal_and_advance("null") 
                ? json_null()
                : return_with_error_code(error_code::illegal_literal); 
        }

        bool compare_literal_and_advance(string_view literal)
        {
            // compare, string_view::substr will check length automatically.
            if (m_cur.compare(0, literal.size(), literal) != 0)
            {
                return false;
            }
            advance_unchecked(literal.size());
            return true;
        }
        
        json_value parse_object()
        {
            advance_unchecked(1); // eat '{'

            skip_whitespace();

            if (current() == '}')
            {
                advance_unchecked(1); // eat '}'
                return json_value(json_object());
            }
            else if (current() == '\"')
            {
                // parse key-value pair
                json_object obj;

                while (1) 
                {
                    auto key = parse_string();

                    if (!key)
                    {
                        // return return_with_error_code(error_code::illegal_object);
                        return key;
                    }

                    skip_whitespace();

                    if (!match_and_advance(':'))
                    {
                        return return_with_error_code(error_code::illegal_object);
                    }
                    
                    skip_whitespace();

                    auto value = parse_value();

                    if (!value)
                    {
                        // return return_with_error_code(error_code::illegal_object);
                        return value;
                    }

                    obj.emplace(std::move(*key.as_ptr<json_string>()), std::move(value));

                    skip_whitespace();

                    if (current() == '}')
                    {
                        advance_unchecked(1); // eat '}'
                        return json_value(std::move(obj));
                    }

                    if (!match_and_advance(','))
                    {
                        return return_with_error_code(error_code::illegal_object);
                    }

                    skip_whitespace();
                }
            }
            else
            {
                return return_with_error_code(error_code::illegal_object);
            }
        }

        json_value parse_string()
        {
            advance_unchecked(1); // eat '"'

            json_string s;

            while (1)
            {
                if (m_cur.empty())
                {
                    return return_with_error_code(error_code::illegal_string);
                }

                char ch = current();

                if (ch == '"')
                {
                    advance_unchecked(1); // eat '"'
                    return json_value(std::move(s));
                }

                if (ch == '\\')
                {
                    advance_unchecked(1); // eat '\\'

                    if (m_cur.empty())
                    {
                        return return_with_error_code(error_code::illegal_string);
                    }

                    switch (current())
                    {
                        case '"': s += '"'; break;    // quotation
                        case '\\': s += '\\'; break;  // reverse solidus
                        case '/': s += '/'; break;    // solidus
                        case 'b': s += '\b'; break;   // backspace
                        case 'f': s += '\f'; break;   // formfeed
                        case 'n': s += '\n'; break;   // linefeed
                        case 'r': s += '\r'; break;   // carriage return
                        case 't': s += '\t'; break;   // horizontal tab
                        case 'u': {
                            // https://codebrowser.dev/llvm/llvm/lib/Support/JSON.cpp.html#_ZN4llvm4json12_GLOBAL__N_110encodeUtf8EjRNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
                            auto parse_4_hex = [this](string_view sv) -> optional<uint16_t> {
                                if (sv.size() < 4 || !is_unicode<4>(sv.data()))
                                {
                                    return nullopt;
                                }
                                auto res = decode_unicode_from_char<4>(sv.data());  
                                advance_unchecked(4);
                                return res;
                            };

                            // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
                            auto invalid = [&] { s.append({'\xef', '\xbf', '\xbd'}); };

                            uint16_t first;

                            if (auto op = parse_4_hex(m_cur.substr(1, 4)); !op)
                            {
                                return return_with_error_code(error_code::illegal_unicode);
                            }
                            else
                            {
                                first = *op;
                            }

                            while (1)
                            {
                                // basic multilingual plane, BMP(U+0000 - U+FFFF)
                                if (first < 0xD800 || first >= 0xE000) [[likely]] 
                                {
                                    encode_unicode_to_utf8(std::back_inserter(s), first);
                                    break;
                                }
                                if (first >= 0xDC00) [[unlikely]] 
                                {
                                    invalid();
                                    break;
                                }
                                if (m_cur.size() < 2 + 1 || peek(1) != '\\' || peek(2) != 'u') [[unlikely]] 
                                {
                                    invalid();
                                    break;
                                }
                                advance_unchecked(2); // skip "\u"

                                uint16_t second;

                                if (auto op = parse_4_hex(m_cur.substr(1, 4)); !op)
                                {
                                    return return_with_error_code(error_code::illegal_unicode);
                                }
                                else
                                {
                                    second = *op;
                                }

                                if (second < 0xDC || second >= 0xE000) [[unlikely]]
                                {
                                    invalid();
                                    first = second;
                                    continue;
                                }
                                uint32_t codepoint = 0x10000 | ((first - 0xD800) << 10) | (second - 0xDC00);
                                encode_unicode_to_utf8(std::back_inserter(s), codepoint);
                                break;
                            }
                            break;
                        }   // 4 hex digits
                        default: return return_with_error_code(error_code::illegal_string);
                    }

                }
                else
                {
                    s += ch;
                }

                advance_unchecked(1);
            }
        }

        json_value parse_number()
        {
            char ch = current();

            // We use std::from_chars to help us parse number.
            // This if-else branch is not necessary, but we want use it 
            // to distinct the tow error cases.
            if (ch == '-' || isdigit(ch))
            {
                auto startptr = m_cur.data();

                while (m_cur.size() && valid_number_character(current()))
                {
                    advance_unchecked(1);
                }

                auto endptr = m_cur.data();

                // Since leading zeroes and 0x, 0b, 0o is not permitted, so if 
                // a number started with 0, we assume it is a floating number.
                if (startptr[0] != '0')
                {
                    // Try parse as integral first.
                    if (auto value = from_chars_to_optional<json_number::int_type>(startptr, endptr); value)
                    {
                        return json_number(*value);
                    }

                    // Try parse as unsigned integral second.
                    if (auto value = from_chars_to_optional<json_number::uint_type>(startptr, endptr); value)
                    {
                        return json_number(*value);
                    }
                }
                else if (startptr + 1 == endptr) // "0"
                {
                    return json_number(0);
                } 
                else if (startptr[1] != '.') // Only "0." is allowed
                {
                    return return_with_error_code(error_code::illegal_number);
                }

                // Try parse as floating last.
                if (auto value = from_chars_to_optional<json_number::float_type>(startptr, endptr); value)
                {
                    return json_number(*value);
                }    

                return return_with_error_code(error_code::illegal_number);
            }
            else
            {
                return return_with_error_code(error_code::unknown_character);
            }
        }

        // Whitespace is consist of (' ', '\r', '\n', '\t').
        void skip_whitespace()
        {
            // auto idx = m_cur.find_first_not_of(whitespace_delimiters);
            // m_cur.remove_prefix(idx == m_cur.npos ? m_cur.size() : idx);

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
            for (; m_cur.size() && is_whitespace(current()); advance_unchecked(1));
        }

        static bool valid_number_character(char ch)
        {
            struct valid_character_config
            {
                int operator()(size_t x) const
                {
                    [[assume(x < 256)]];  
                    constexpr std::string_view sv = "-+.eE";
                    return isdigit(x) || sv.contains(x); // x is less than 128 and it can convert to char.
                }
            };

            static auto valid_characters = make_character_table(valid_character_config());

            return valid_characters[ch];

            // if (isdigit(ch))
            // {
            //     return true;
            // }
            // switch (ch)
            // {
            //     case '-':
            //     case '+':
            //     case '.':
            //     case 'e':
            //     case 'E': return true;
            //     default: return false;
            // }
        }

        void advance_unchecked(size_t n)
        { m_cur.remove_prefix(n); }

        // What if we directly add a '\0'(or some other special characters) as sentinel at the end of context?
        char current() const
        { return peek(0); }

        char peek(int n) const
        { return m_cur[n]; }

        json_value return_with_error_code(error_code err)
        { return err;}

        bool match_and_advance(char ch)
        {
            if (m_cur.empty())
            {
                return false;
            }
            bool result = current() == ch;
            advance_unchecked(1);
            return result;
        }
    };

    json_value parse_json(const char* filename)
    { return parser(read_file_contents(filename))(); }

    json_value load(string s)
    { return parser(std::move(s))(); }

    namespace detail
    {
        struct dump_helper
        {
            constexpr static const char* padding_character = "  ";

            template <typename OStream, typename Character>
            static void json_padding(OStream& os, Character character, int count)
            {
                for (int i = 0; i < count; ++i)
                    os << character;
            }

            template <typename OStream>
            void operator()(OStream& os, const json_number& number, int padding) const
            { os << number; }

            template <typename OStream>
            void operator()(OStream& os, const std::unique_ptr<json_string>& stringptr, int padding) const
            {
                auto& string = *stringptr;
                os << '"' << string << '"';
            }

            template <typename OStream>
            void operator()(OStream& os, const std::unique_ptr<json_array>& arrayptr, int padding) const
            {
                auto& arr = *arrayptr;
                os << "[";
                for (std::size_t i = 0; i < arr.size(); ++i)
                {
                    if (i != 0) os << ", ";
                    this->operator()(os, arr[i], 0);
                }
                os << ']';
            }

            template <typename OStream>
            void operator()(OStream& os, const json_boolean& boolean, int padding) const
            { os << (boolean ? "true" : "false"); }

            template <typename OStream>
            void operator()(OStream& os, const json_null&, int padding) const
            { os << "null"; }

            template <typename OStream>
            void operator()(OStream& os, const std::unique_ptr<json_object>& objectptr, int padding) const
            {
                auto& object = *objectptr;
                os << "{\n";
                for (auto it = object.begin(); it != object.end(); ++it)
                {
                    if (it != object.begin()) os << ",\n";
                    json_padding(os, padding_character, padding);
                    os << std::format(R"("{}" : )", it->first);
                    this->operator()(os, it->second, padding + 1);
                }
                os << '\n';
                json_padding(os, padding_character, padding);
                os << '}';
            }

            template <typename OStream>
            void operator()(OStream& os, const json_value& value, int padding) const
            {
                std::visit([&os, padding, this]<typename T>(const T& t)
                {
                    if constexpr (std::is_same_v<T, error_code>)
                    {
                        std::unreachable();
                    }
                    this->operator()(os, t, padding);
                }, value.data());
            }
        };
    } // namespace detail

    string dump(const json_value& val)
    {
        std::stringstream ss;
        detail::dump_helper()(ss, val, 0);
        return ss.str();
    }

} // namespace leviathan::config::json

namespace leviathan::json
{
    using namespace ::leviathan::config::json;
}

#include <format>

template <typename CharT>
struct std::formatter<leviathan::json::json_value, CharT> 
{
    static_assert(std::is_same_v<CharT, char>, "Only support char");

    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    { return ctx.begin(); }

    template <typename FmtContext>
    typename FmtContext::iterator format(const leviathan::json::json_value& val, FmtContext& ctx) const
    {
        std::basic_string<CharT> result = leviathan::json::dump(val);
        return std::ranges::copy(result, ctx.out()).out;
    }
};