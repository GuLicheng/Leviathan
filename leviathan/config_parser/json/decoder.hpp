#pragma once

#include "value.hpp"
#include "../parse_context.hpp"
#include "../common.hpp"

namespace leviathan::config::json
{

class decoder
{
    static bool valid_number_character(char ch)
    {
        struct valid_character_config
        {
            int operator()(size_t x) const
            {
                [[assume(x < 256)]];  
                constexpr std::string_view sv = "-+.eE";
                return ::isdigit(x) || sv.contains(x); // x is less than 128 and it can convert to char.
            }
        };

        static auto valid_characters = make_character_table(valid_character_config());
        return valid_characters[ch];
    }

    static value make_error_code(error_code ec)
    {
        return ec;
    }

    value parse_array_or_object()
    {
        m_ctx.skip_whitespace();

        switch (m_ctx.current())
        {
            case '{': return parse_object();
            case '[': return parse_array();
            default: return make_error_code(error_code::error_payload);
        }
    }

public:

    decoder(std::string_view context) : m_ctx(context) { }

    value parse_null()
    {
        return m_ctx.match_literal_and_advance("null") 
            ? null()
            : make_error_code(error_code::illegal_literal); 
    }

    value parse_false()
    {
        return m_ctx.match_literal_and_advance("false") 
            ? boolean(false) 
            : make_error_code(error_code::illegal_literal); 
    }

    value parse_true()
    {
        return m_ctx.match_literal_and_advance("true") 
            ? boolean(true) 
            : make_error_code(error_code::illegal_literal); 
    }

    value parse_value()
    {
        m_ctx.skip_whitespace(); // necessary?

        if (m_ctx.is_at_end())
        {
            return make_error_code(error_code::eof_error);
        }

        switch (m_ctx.current())
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

    value parse_array()
    {
        m_ctx.advance_unchecked(1); // eat '['
        m_ctx.skip_whitespace();

        array arr;

        // FIXME: match -> current
        if (m_ctx.current() == ']')
        {
            m_ctx.advance_unchecked(1); // eat ']'
            return arr;
        }   
        else 
        {
            while (1)
            {
                auto value = parse_value();

                if (!value)
                {
                    return value; 
                }

                arr.emplace_back(std::move(value));
                m_ctx.skip_whitespace();

                if (m_ctx.current() == ']')
                {
                    m_ctx.advance_unchecked(1); // eat ']'
                    return arr;
                }

                if (!m_ctx.match_and_advance(','))
                {
                    return make_error_code(error_code::illegal_array);
                }

                m_ctx.skip_whitespace();
            }
        }
    }

    value parse_object()
    {
        m_ctx.advance_unchecked(1); // eat '{'
        m_ctx.skip_whitespace();

        if (m_ctx.current() == '}')
        {
            m_ctx.advance_unchecked(1); // eat '}'
            return object();
        }
        else if (m_ctx.current() == '\"')
        {
            // parse key-value pair
            object obj;

            while (1) 
            {
                auto key = parse_string();

                if (!key)
                {
                    return key;
                }

                m_ctx.skip_whitespace();

                if (!m_ctx.match_and_advance(':'))
                {
                    return make_error_code(error_code::illegal_object);
                }
                
                m_ctx.skip_whitespace();
                auto value = parse_value();

                if (!value)
                {
                    return value;
                }

                obj.emplace(std::move(*key.as_ptr<string>()), std::move(value));
                m_ctx.skip_whitespace();

                if (m_ctx.current() == '}')
                {
                    m_ctx.advance_unchecked(1); // eat '}'
                    return obj;
                }

                if (!m_ctx.match_and_advance(','))
                {
                    return make_error_code(error_code::illegal_object);
                }

                m_ctx.skip_whitespace();
            }
        }
        else
        {
            return make_error_code(error_code::illegal_object);
        }
    }

    value parse_string()
    {
        m_ctx.advance_unchecked(1); // eat '"'
        string s;

        while (1)
        {
            if (m_ctx.is_at_end())
            {
                return make_error_code(error_code::illegal_string);
            }

            char ch = m_ctx.current();

            if (ch == '"')
            {
                m_ctx.advance_unchecked(1); // eat '"'
                return make_json<string>(std::move(s));
            }

            if (ch == '\\')
            {
                m_ctx.advance_unchecked(1); // eat '\\'

                if (m_ctx.is_at_end())
                {
                    return make_error_code(error_code::illegal_string);
                }

                switch (m_ctx.current())
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
                        auto parse_4_hex = [this](std::string_view sv) -> optional<uint16_t> {
                            if (sv.size() < 4 || !is_unicode<4>(sv.data()))
                            {
                                return nullopt;
                            }
                            auto res = decode_unicode_from_char<4>(sv.data());  
                            m_ctx.advance_unchecked(4);
                            return res;
                        };

                        // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
                        auto invalid = [&] { s.append({'\xef', '\xbf', '\xbd'}); };

                        uint16_t first;

                        if (auto op = parse_4_hex(m_ctx.slice(1, 4)); !op)
                        {
                            return make_error_code(error_code::illegal_unicode);
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
                            if (m_ctx.size() < 2 + 1 || m_ctx.peek(1) != '\\' || m_ctx.peek(2) != 'u') [[unlikely]] 
                            {
                                invalid();
                                break;
                            }
                            m_ctx.advance_unchecked(2); // skip "\u"

                            uint16_t second;

                            if (auto op = parse_4_hex(m_ctx.slice(1, 4)); !op)
                            {
                                return make_error_code(error_code::illegal_unicode);
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
                    default: return make_error_code(error_code::illegal_string);
                }

            }
            else
            {
                s += ch;
            }

            m_ctx.advance_unchecked(1);
        }
    }

    value parse_number()
    {
        char ch = m_ctx.current();

        // We use std::from_chars to help us parse number.
        // This if-else branch is not necessary, but we want use it 
        // to distinct the tow error cases.
        if (ch == '-' || ::isdigit(ch))
        {
            auto startptr = m_ctx.data();

            while (m_ctx.size() && valid_number_character(m_ctx.current()))
            {
                m_ctx.advance_unchecked(1);
            }

            auto endptr = m_ctx.data();

            // Since leading zeroes and 0x, 0b, 0o is not permitted, so if 
            // a number started with 0, we assume it is a floating number.
            if (startptr[0] != '0')
            {
                // Try parse as integral first.
                if (auto value = from_chars_to_optional<number::int_type>(startptr, endptr); value)
                {
                    return number(*value);
                }

                // Try parse as unsigned integral second.
                if (auto value = from_chars_to_optional<number::uint_type>(startptr, endptr); value)
                {
                    return number(*value);
                }
            }
            else if (startptr + 1 == endptr) // "0"
            {
                return number(0);
            } 
            else if (startptr[1] != '.') // Only "0." is allowed
            {
                return make_error_code(error_code::illegal_number);
            }

            // Try parse as floating last.
            if (auto value = from_chars_to_optional<number::float_type>(startptr, endptr); value)
            {
                return number(*value);
            }    

            return make_error_code(error_code::illegal_number);
        }
        else
        {
            return make_error_code(error_code::unknown_character);
        }
    }

    value parse()
    {
        return this->operator()();
    }

    value operator()()
    {
        // "A JSON payload should be an object or array."
        // For debugging, we just parse value.
        // auto root parse_array_or_object();
        auto root = parse_value();
        
        if (!root)
        {
            return root;
        }

        m_ctx.skip_whitespace();
        // return is_over() ? root : make_error_code(error_code::multi_value);
        if (m_ctx.is_at_end())
        {
            return root;
        }
        else
        {
            return make_error_code(error_code::multi_value);
        }
    }

    value operator()(std::string_view context)
    {
        m_ctx = context;
        return this->operator()();
    }

private:

    parse_context m_ctx;
};

inline value loads(std::string source)
{
    return decoder(source)();
}

inline value load(const char* filename)
{
    return loads(leviathan::read_file_context(filename));
}

} // namespace leviathan::config::json

namespace leviathan::config
{

template <>
struct value_parser<json::value>
{
    static json::value operator()(std::string source)
    {
        return json::decoder(source).parse_value();
    }
};

}
