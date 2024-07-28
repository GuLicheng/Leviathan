#pragma once

#if 0 && JSON_DEPRECATED

#include "common.hpp"
#include "parser.hpp"
#include "json_value.hpp"
#include <string_view>

namespace leviathan::config::json
{
    
class json_decoder
{

    std::string_view m_context;

    // What if we directly add a '\0'(or some other special characters) as sentinel at the end of context?
    char current() const
    {
        return peek(0);
    }

    bool match_and_advance(char ch)
    {
        if (m_context.empty())
        {
            return false;
        }

        bool result = current() == ch;
        advance_unchecked(1);
        return result;
    }

    char peek(int n) const
    {
        return m_context[n];
    }

    void advance_unchecked(size_t n)
    {
        m_context.remove_prefix(n);
    }

    bool match_literal_and_advance(std::string_view literal)
    {
        // compare, string_view::substr will check length automatically.
        if (m_context.compare(0, literal.size(), literal) != 0)
        {
            return false;
        }

        advance_unchecked(literal.size());
        return true;
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

    void skip_whitespace()
    {
        // auto idx = m_context.find_first_not_of(whitespace_delimiters);
        // m_context.remove_prefix(idx == m_context.npos ? m_context.size() : idx);
        struct whitespace_config
        {
            constexpr static int operator()(size_t i) 
            {
                [[assume(i < 256)]];
                constexpr std::string_view sv = " \r\n\t";
                return sv.contains(i);
            }
        };

        static auto whitespaces = make_character_table(whitespace_config());

        auto is_whitespace = [](char ch) { return whitespaces[ch]; };
        for (; m_context.size() && is_whitespace(current()); advance_unchecked(1));
    }

    bool is_over() const
    {
        return m_context.empty();
    }

    static json_value make_error_code(error_code ec)
    {
        return ec;
    }

    json_value parse_null()
    {
        return match_literal_and_advance("null") 
            ? json_null()
            : make_error_code(error_code::illegal_literal); 
    }

    json_value parse_false()
    {
        return match_literal_and_advance("false") 
            ? json_boolean(false) 
            : make_error_code(error_code::illegal_literal); 
    }

    json_value parse_true()
    {
        return match_literal_and_advance("true") 
            ? json_boolean(true) 
            : make_error_code(error_code::illegal_literal); 

        // Advance and check rest characters may faster.
        // advance_unchecked(1); return match_literal_and_advance("rue") ...
    }

    json_value parse_array_or_object()
    {
        skip_whitespace();

        switch (current())
        {
            case '{': return parse_object();
            case '[': return parse_array();
            default: return make_error_code(error_code::error_payload);
        }
    }

    json_value parse_value()
    {
        skip_whitespace(); // necessary?

        if (is_over())
        {
            return make_error_code(error_code::eof_error);
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
                    return make_error_code(error_code::illegal_array);
                }

                skip_whitespace();
            }
        }
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
                    return key;
                }

                skip_whitespace();

                if (!match_and_advance(':'))
                {
                    return make_error_code(error_code::illegal_object);
                }
                
                skip_whitespace();
                auto value = parse_value();

                if (!value)
                {
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
                    return make_error_code(error_code::illegal_object);
                }

                skip_whitespace();
            }
        }
        else
        {
            return make_error_code(error_code::illegal_object);
        }
    }

    json_value parse_string()
    {
        advance_unchecked(1); // eat '"'
        json_string s;

        while (1)
        {
            if (m_context.empty())
            {
                return make_error_code(error_code::illegal_string);
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

                if (m_context.empty())
                {
                    return make_error_code(error_code::illegal_string);
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
                        auto parse_4_hex = [this](std::string_view sv) -> optional<uint16_t> {
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

                        if (auto op = parse_4_hex(m_context.substr(1, 4)); !op)
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
                            if (m_context.size() < 2 + 1 || peek(1) != '\\' || peek(2) != 'u') [[unlikely]] 
                            {
                                invalid();
                                break;
                            }
                            advance_unchecked(2); // skip "\u"

                            uint16_t second;

                            if (auto op = parse_4_hex(m_context.substr(1, 4)); !op)
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
            auto startptr = m_context.data();

            while (m_context.size() && valid_number_character(current()))
            {
                advance_unchecked(1);
            }

            auto endptr = m_context.data();

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
                return make_error_code(error_code::illegal_number);
            }

            // Try parse as floating last.
            if (auto value = from_chars_to_optional<json_number::float_type>(startptr, endptr); value)
            {
                return json_number(*value);
            }    

            return make_error_code(error_code::illegal_number);
        }
        else
        {
            return make_error_code(error_code::unknown_character);
        }
    }

public:

    json_decoder(std::string_view context) : m_context(context) { }

    json_value operator()()
    {
        // "A JSON payload should be an object or array."
        // For debugging, we just parse value.
        // auto root parse_array_or_object();
        auto root = parse_value();
        
        if (!root)
        {
            return root;
        }

        skip_whitespace();
        // return is_over() ? root : make_error_code(error_code::multi_value);
        if (is_over())
        {
            return root;
        }
        else
        {
            return make_error_code(error_code::multi_value);
        }
    }

private:

};

inline json_value loads(std::string source)
{
    return parser<json_decoder>()(std::move(source));
}

inline json_value load(const char* filename)
{
    return loads(leviathan::read_file_context(filename));
}

} // namespace leviathan::config::json

#endif

#include "parse_context.hpp"
#include "common.hpp"
#include "json_value.hpp"
#include "../parser.hpp"

namespace leviathan::config::json
{

inline json_value make_error_code(error_code ec)
{
    return ec;
}

// ParseContext is not necessary, which is just for deducing template initialize.
template <typename T, typename ParseContext> 
struct json_decoder;

template <typename ParseContext>
struct json_decoder<json_null, ParseContext>
{
    static json_value operator()(ParseContext& ctx)
    {
        return ctx.match_literal_and_advance("null") 
            ? json_null()
            : make_error_code(error_code::illegal_literal); 
    }
};

template <typename ParseContext>
struct json_decoder<json_boolean, ParseContext>
{
    static json_value operator()(ParseContext& ctx, std::true_type)
    {
        return ctx.match_literal_and_advance("true") 
             ? json_boolean(true)
             : make_error_code(error_code::illegal_literal); 
    }

    static json_value operator()(ParseContext& ctx, std::false_type)
    {
        return ctx.match_literal_and_advance("false") 
             ? json_boolean(false)
             : make_error_code(error_code::illegal_literal); 
    }
};

template <typename ParseContext>
struct json_decoder<json_string, ParseContext>
{
    static json_value operator()(ParseContext& ctx)
    {
        ctx.advance_unchecked(1); // eat '"'
        json_string s;

        while (1)
        {
            if (ctx.is_at_end())
            {
                return make_error_code(error_code::illegal_string);
            }

            char ch = ctx.current();

            if (ch == '"')
            {
                ctx.advance_unchecked(1); // eat '"'
                return json_value(std::move(s));
            }

            if (ch == '\\')
            {
                ctx.advance_unchecked(1); // eat '\\'

                if (ctx.is_at_end())
                {
                    return make_error_code(error_code::illegal_string);
                }

                switch (ctx.current())
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
                        auto parse_4_hex = [&](std::string_view sv) -> optional<uint16_t> {
                            if (sv.size() < 4 || !is_unicode<4>(sv.data()))
                            {
                                return nullopt;
                            }
                            auto res = decode_unicode_from_char<4>(sv.data());  
                            ctx.advance_unchecked(4);
                            return res;
                        };

                        // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
                        auto invalid = [&] { s.append({'\xef', '\xbf', '\xbd'}); };

                        uint16_t first;

                        if (auto op = parse_4_hex(ctx.slice(1, 4)); !op)
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
                            if (ctx.size() < 2 + 1 || ctx.peek(1) != '\\' || ctx.peek(2) != 'u') [[unlikely]] 
                            {
                                invalid();
                                break;
                            }
                            
                            ctx.advance_unchecked(2); // skip "\u"
                            uint16_t second;

                            if (auto op = parse_4_hex(ctx.slice(1, 4)); !op)
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

            ctx.advance_unchecked(1);
        }
    }
};

template <typename ParseContext>
struct json_decoder<json_number, ParseContext>
{
    static json_value operator()(ParseContext& ctx)
    {
        char ch = ctx.current();

        // We use std::from_chars to help us parse number.
        // This if-else branch is not necessary, but we want use it 
        // to distinct the tow error cases.
        if (ch == '-' || ::isdigit(ch))
        {
            auto startptr = ctx.data();

            while (ctx.size() && valid_number_character(ctx.current()))
            {
                ctx.advance_unchecked(1);
            }

            auto endptr = ctx.data();

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
                return make_error_code(error_code::illegal_number);
            }

            // Try parse as floating last.
            if (auto value = from_chars_to_optional<json_number::float_type>(startptr, endptr); value)
            {
                return json_number(*value);
            }    

            return make_error_code(error_code::illegal_number);
        }
        else
        {
            return make_error_code(error_code::unknown_character);
        }
    }
};

template <typename ParseContext>
struct json_decoder<json_array, ParseContext>
{
    static json_value operator()(ParseContext& ctx)
    {
        ctx.advance_unchecked(1); // eat '['
        ctx.skip_whitespace();

        json_array arr;

        if (ctx.current() == ']')
        {
            ctx.advance_unchecked(1); // eat ']'
            return json_value(std::move(arr));
        }   
        else 
        {
            while (1)
            {
                // auto value = parse_value();
                auto value = json_decoder<json_value, ParseContext>()(ctx);

                if (!value)
                {
                    return value; 
                }

                arr.emplace_back(std::move(value));
                ctx.skip_whitespace();

                if (ctx.current() == ']')
                {
                    ctx.advance_unchecked(1); // eat ']'
                    return json_value(std::move(arr));
                }

                if (!ctx.match_and_advance(','))
                {
                    return make_error_code(error_code::illegal_array);
                }

                ctx.skip_whitespace();
            }
        }
    }
};

template <typename ParseContext>
struct json_decoder<json_value, ParseContext>
{
    static json_value operator()(ParseContext& ctx)
    {
        ctx.skip_whitespace(); // necessary?

        if (ctx.is_at_end())
        {
            return make_error_code(error_code::eof_error);
        }

        switch (ctx.current())
        {
            case 't': return json_decoder<json_boolean, ParseContext>()(ctx, std::true_type());
            case 'n': return json_decoder<json_null, ParseContext>()(ctx);
            case 'f': return json_decoder<json_boolean, ParseContext>()(ctx, std::false_type());
            case '[': return json_decoder<json_array, ParseContext>()(ctx);
            case '{': return json_decoder<json_object, ParseContext>()(ctx);
            case '"': return json_decoder<json_string, ParseContext>()(ctx);
            default: return json_decoder<json_number, ParseContext>()(ctx);
        }
    }
};

template <typename ParseContext>
struct json_decoder<json_object, ParseContext>
{
    static json_value operator()(ParseContext& ctx)
    {
        ctx.advance_unchecked(1); // eat '{'
        ctx.skip_whitespace();

        if (ctx.current() == '}')
        {
            ctx.advance_unchecked(1); // eat '}'
            return json_value(json_object());
        }
        else if (ctx.current() == '\"')
        {
            // parse key-value pair
            json_object obj;

            while (1) 
            {
                // auto key = parse_string();
                auto key = json_decoder<json_string, ParseContext>()(ctx);

                if (!key)
                {
                    return key;
                }

                ctx.skip_whitespace();

                if (!ctx.match_and_advance(':'))
                {
                    return make_error_code(error_code::illegal_object);
                }
                
                ctx.skip_whitespace();
                // auto value = parse_value();
                auto value = json_decoder<json_value, ParseContext>()(ctx);

                if (!value)
                {
                    return value;
                }

                obj.emplace(std::move(*key.template as_ptr<json_string>()), std::move(value));
                ctx.skip_whitespace();

                if (ctx.current() == '}')
                {
                    ctx.advance_unchecked(1); // eat '}'
                    return json_value(std::move(obj));
                }

                if (!ctx.match_and_advance(','))
                {
                    return make_error_code(error_code::illegal_object);
                }

                ctx.skip_whitespace();
            }
        }
        else
        {
            return make_error_code(error_code::illegal_object);
        }
    }
};

}

namespace leviathan
{

template <typename CharT> 
struct parser<json::json_value, CharT>
{
    template <typename ParseContext>
    static constexpr json::json_value parse_array_or_object(ParseContext& ctx)
    {
        ctx.skip_whitespace();

        switch (ctx.current())
        {
            case '{': return json::json_decoder<json::json_object, ParseContext>()(ctx);
            case '[': return json::json_decoder<json::json_array, ParseContext>()(ctx);
            default: return json::make_error_code(json::error_code::error_payload);
        }
    }

    static constexpr json::json_value operator()(const std::string& ctx)
    {
        // "A JSON payload should be an object or array."
        // For debugging, we just parse value.
        config::parse_context context = std::string_view(ctx);
        auto root = parse_array_or_object(context);
        
        if (!root)
        {
            return root;
        }

        context.skip_whitespace();

        if (context.is_at_end())
        {
            return root;
        }
        else
        {
            return json::make_error_code(json::error_code::multi_value);
        }
    }
};

} // namespace leviathan

namespace leviathan::json
{

inline json_value loads(std::string source)
{
    return parser<json::json_value, char>()(source);
}

inline json_value load(const char* filename)
{
    return loads(leviathan::read_file_context(filename));
}

} // namespace leviathan::json


