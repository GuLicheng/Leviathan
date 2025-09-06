#pragma once

#include <leviathan/config_parser/common.hpp>
#include <leviathan/config_parser/context.hpp>
#include <leviathan/config_parser/json/value.hpp>

namespace cpp::config::json::detail
{

template <typename Context>
class string_decoder
{
    using char_type = typename Context::char_type;

    static void decode_unicode(Context& ctx, string& result)
    {
        // https://codebrowser.dev/llvm/llvm/lib/Support/JSON.cpp.html#_ZN4llvm4json12_GLOBAL__N_110encodeUtf8EjRNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
        // https://llvm.org/doxygen/JSON_8cpp_source.html
        ctx.advance(2); // eat '\u'

        auto parse4hex = [&ctx](std::basic_string_view<char_type> sv) -> uint16_t 
        {
            if (ctx.size() < 4 || !is_unicode<4>(sv.data()))
            {
                throw std::runtime_error("Invalid unicode sequence");
            }

            auto res = decode_unicode_from_char<4>(ctx.to_string_view().data());  
            ctx.advance(4);
            return res;
        };

        auto invalid = [&]()  {
            // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
            result.append({'\xef', '\xbf', '\xbd'});
        };

        uint16_t first = parse4hex(ctx.to_string_view().substr(0, 4));

        while (1)
        {
            if (first < 0xD800 || first >= 0xE000) [[likely]] 
            {
                encode_unicode_to_utf8(std::back_inserter(result), first);
                break;
            }

            if (first >= 0xE000) [[unlikely]] 
            {
                invalid();
                break;
            }
            
            if (!ctx.match("\\u", false)) [[unlikely]] 
            {
                invalid();
                break;
            }

            ctx.advance(2); // skip "\u"
            uint16_t second = parse4hex(ctx.to_string_view().substr(0, 4));

            if (second < 0xDC00 || second >= 0xE000) [[unlikely]] 
            {
                invalid();
                first = second;
                continue;
            }

            uint32_t codepoint = 0x10000 | ((first - 0xD800) << 10) | (second - 0xDC00);
            encode_unicode_to_utf8(std::back_inserter(result), codepoint);
            break;
        }
    }

    static bool illegal_character(char_type ch)
    {
        return ch != '\n' && ch != '\r';
    }

    static string decode_json_string(Context& ctx)
    {
        string result;

        if (!ctx.match('"', true))
        {
            throw std::runtime_error("Expected '\"' at the beginning of string.");
        }

        for (; !ctx.eof(); )
        {
            if (ctx.current() == '\\')
            {
                switch (ctx.next())
                {
                    case '"': result += '"'; ctx.advance(2); break;
                    case '\\': result += '\\'; ctx.advance(2); break;
                    case '/': result += '/'; ctx.advance(2); break;
                    case 'b': result += '\b'; ctx.advance(2); break;
                    case 'f': result += '\f'; ctx.advance(2); break;
                    case 'n': result += '\n'; ctx.advance(2); break;
                    case 'r': result += '\r'; ctx.advance(2); break;
                    case 't': result += '\t'; ctx.advance(2); break;
                    case 'u': decode_unicode(ctx, result); break;
                    default: throw std::runtime_error("Invalid escape sequence.");
                }
            }
            else if (ctx.current() == '"')
            {
                ctx.advance(1); // eat '"'
                break;
            }
            else if (!illegal_character(ctx.current()))
            {
                throw std::runtime_error("Illegal character in string.");
            }
            else
            {
                result += ctx.current();
                ctx.advance(1);
            }
        }
        return result;
    }

public:

    static string operator()(Context& ctx)
    {
        return decode_json_string(ctx);
    }
};

template <typename Context>
class number_decoder
{
    using char_type = typename Context::char_type;

    static bool valid_number_character(char_type ch)
    {
        static std::basic_string_view<char_type> dictionary = "0123456789.eE-+";
        return dictionary.contains(ch);
    }

    static bool check_leading(std::basic_string_view<char_type> sv)
    {
        static std::basic_string_view<char_type> invalid_leading = ".eE+";

        if (sv.size())
        {
            if (invalid_leading.contains(sv[0]))
            {
                return false;
            }

            if (sv.size() > 1 && sv[0] == '0')
            {
                return sv[1] == '.' || sv[1] == 'e' || sv[1] == 'E';
            }
        }

        return true;
    }

    static number decode_json_number(Context& ctx)
    {
        assert(!ctx.eof());

        size_t count = 0;
        for (; count < ctx.size() && valid_number_character(ctx.peek(count)); ++count);

        auto sv = ctx.to_string_view().substr(0, count);

        if (!check_leading(sv))
        {
            throw std::runtime_error("Invalid leading character is not allowed in number.");
        }

        using SignedInteger = typename number::int_type;
        using UnsignedInteger = typename number::uint_type;
        using FloatingPoint = typename number::float_type;
        constexpr auto ReturnPolicy = error_policy::optional;

        // Try to parse as integer first, then unsigned integer, finally floating point.
        if (auto result1 = caster<SignedInteger, ReturnPolicy>()(sv); result1)
        {
            ctx.advance(count);
            return number(*result1);
        }
        else if (auto result2 = caster<UnsignedInteger, ReturnPolicy>()(sv); result2)
        {
            ctx.advance(count);
            return number(*result2);
        }
        else if (auto result3 = caster<FloatingPoint, ReturnPolicy>()(sv); result3)
        {
            ctx.advance(count);
            return number(*result3);
        }
        else
        {
            throw std::runtime_error("Invalid number format.");
        }
    }

public:

    static number operator()(Context& ctx)
    {
        return decode_json_number(ctx);
    }
};

template <typename Context>
class decode2
{
    Context m_ctx;

    value decode_null()
    {
        if (m_ctx.match("null", true))
        {
            return null();
        }
        throw std::runtime_error("Invalid literal, expected 'null'.");
    }

    value decode_boolean()
    {
        if (m_ctx.match("true", true))
        {
            return boolean(true);
        }
        else if (m_ctx.match("false", true))
        {
            return boolean(false);
        }
        throw std::runtime_error("Invalid literal, expected 'true' or 'false'.");
    }

    value decode_string()
    {
        return string_decoder<Context>()(m_ctx);
    }

    value decode_number()
    {
        return number_decoder<Context>()(m_ctx);
    }

    value decode_array()
    {
        m_ctx.match('[', true); // eat '['
        m_ctx.skip_whitespace();

        array arr;

        if (m_ctx.match(']', true))
        {
            return arr;
        }   
        else 
        {
            while (1)
            {
                auto val = decode_value();
                arr.emplace_back(std::move(val));
                m_ctx.skip_whitespace();

                if (m_ctx.match(']', true))
                {
                    return arr;
                }

                if (!m_ctx.match(',', true))
                {
                    throw std::runtime_error("Invalid array, expected ',' or ']'.");
                }

                m_ctx.skip_whitespace();
            }
        }
    }

    value decode_object()
    {
        if (!m_ctx.match('{', true))
        {
            throw std::runtime_error("Expected '{' at the beginning of object.");
        }

        m_ctx.skip_whitespace();

        if (m_ctx.match('}', true))
        {
            return object();
        }
        else 
        {
            // parse key-value pair
            object obj;

            while (1) 
            {
                // The inner loop may not ensure the next key-value pair
                // such as {"key" : 1, is illegal, so we must check the first character.
                if (!m_ctx.match('"', false))
                {
                    throw std::runtime_error("Invalid object, expected string as key.");
                }

                auto key = decode_string();

                m_ctx.skip_whitespace();

                if (!m_ctx.match(':', true))
                {
                    throw std::runtime_error("Invalid object, expected ':' after key.");
                }
                
                auto val = decode_value();
                obj.emplace(std::move(key.template as<string>()), std::move(val));
                m_ctx.skip_whitespace();

                if (m_ctx.match('}', true))
                {
                    return obj;
                }

                if (!m_ctx.match(',', true))
                {
                    throw std::runtime_error("Invalid object, expected ',' or '}'.");
                }

                m_ctx.skip_whitespace();
            }
        }
    }

    value decode_value()
    {
        m_ctx.skip_whitespace();

        if (m_ctx.eof())
        {
            throw std::runtime_error("Unexpected end of input.");
        }

        switch (m_ctx.current())
        {
            case 'n': return decode_null();
            case 't':
            case 'f': return decode_boolean();
            case '"': return decode_string();
            case '[': return decode_array();
            case '{': return decode_object();
            default: return decode_number();
        }
    }

public:

    decode2(std::string_view sv) : m_ctx(sv) { }

    value operator()()
    {
        m_ctx.skip_whitespace();
        auto result = decode_value();
        m_ctx.skip_whitespace();

        if (!m_ctx.eof())
        {
            throw std::runtime_error("Trailing characters after JSON value.");
        }
        return result;
    }

};

} // namespace detail

namespace cpp::config::json
{

inline constexpr auto loads = [](std::string source) static 
{
    return detail::decode2<basic_context<char>>(source)();
};

inline constexpr auto load = [](const char* filename) static 
{
    return loads(read_file_context(filename));
};

} // namespace cpp::config::json

namespace cpp::config::json::literal
{

inline value operator""_json(const char* str, size_t len)
{
    return loads(std::string(str, len));
}

}