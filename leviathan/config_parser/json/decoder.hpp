// #pragma once

// #include <leviathan/config_parser/common.hpp>
// #include <leviathan/config_parser/parse_context.hpp>
// #include <leviathan/config_parser/context.hpp>
// #include <leviathan/config_parser/json/value.hpp>

// namespace cpp::config::json::detail
// {

// template <typename Context>
// class string_decoder
// {
//     using char_type = typename Context::char_type;

//     static void decode_unicode(Context& ctx, string& result)
//     {
//         // https://codebrowser.dev/llvm/llvm/lib/Support/JSON.cpp.html#_ZN4llvm4json12_GLOBAL__N_110encodeUtf8EjRNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
//         // https://llvm.org/doxygen/JSON_8cpp_source.html
//         ctx.advance(2); // eat '\u'

//         auto parse4hex = [&ctx](std::basic_string_view<char_type> sv) -> uint16_t 
//         {
//             if (ctx.size() < 4 || !is_unicode<4>(sv.data()))
//             {
//                 throw std::runtime_error("Invalid unicode sequence");
//             }

//             auto res = decode_unicode_from_char<4>(ctx.to_string_view().data());  
//             ctx.advance(4);
//             return res;
//         };

//         auto invalid = [&]()  {
//             // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
//             result.append({'\xef', '\xbf', '\xbd'});
//         };

//         uint16_t first = parse4hex(ctx.to_string_view().substr(0, 4));

//         while (1)
//         {
//             if (first < 0xD800 || first >= 0xE000) [[likely]] 
//             {
//                 encode_unicode_to_utf8(std::back_inserter(result), first);
//                 break;
//             }

//             if (first >= 0xE000) [[unlikely]] 
//             {
//                 invalid();
//                 break;
//             }
            
//             if (!ctx.match("\\u", false)) [[unlikely]] 
//             {
//                 invalid();
//                 break;
//             }

//             ctx.advance(2); // skip "\u"
//             uint16_t second = parse4hex(ctx.to_string_view().substr(0, 4));

//             if (second < 0xDC00 || second >= 0xE000) [[unlikely]] 
//             {
//                 invalid();
//                 first = second;
//                 continue;
//             }

//             uint32_t codepoint = 0x10000 | ((first - 0xD800) << 10) | (second - 0xDC00);
//             encode_unicode_to_utf8(std::back_inserter(result), codepoint);
//             break;
//         }
//     }

//     static bool illegal_character(char_type ch)
//     {
//         return ch != '\n' && ch != '\r';
//     }

//     static string decode_json_string(Context& ctx)
//     {
//         string result;

//         if (!ctx.match('"', true))
//         {
//             throw std::runtime_error("Expected '\"' at the beginning of string.");
//         }

//         for (; !ctx.eof(); )
//         {
//             if (ctx.current() == '\\')
//             {
//                 switch (ctx.next())
//                 {
//                     case '"': result += '"'; ctx.advance(2); break;
//                     case '\\': result += '\\'; ctx.advance(2); break;
//                     case '/': result += '/'; ctx.advance(2); break;
//                     case 'b': result += '\b'; ctx.advance(2); break;
//                     case 'f': result += '\f'; ctx.advance(2); break;
//                     case 'n': result += '\n'; ctx.advance(2); break;
//                     case 'r': result += '\r'; ctx.advance(2); break;
//                     case 't': result += '\t'; ctx.advance(2); break;
//                     case 'u': decode_unicode(ctx, result); break;
//                     default: throw std::runtime_error("Invalid escape sequence.");
//                 }
//             }
//             else if (ctx.current() == '"')
//             {
//                 ctx.advance(1); // eat '"'
//                 break;
//             }
//             else if (!illegal_character(ctx.current()))
//             {
//                 throw std::runtime_error("Illegal character in string.");
//             }
//             else
//             {
//                 result += ctx.current();
//                 ctx.advance(1);
//             }
//         }
//         return result;
//     }

// public:

//     static string operator()(Context& ctx)
//     {
//         return decode_json_string(ctx);
//     }
// };

// template <typename Context>
// class number_decoder
// {
//     using char_type = typename Context::char_type;

//     static bool valid_number_character(char_type ch)
//     {
//         static std::basic_string_view<char_type> dictionary = "0123456789.eE-+";
//         return dictionary.contains(ch);
//     }

//     static bool check_leading(std::basic_string_view<char_type> sv)
//     {
//         static std::basic_string_view<char_type> invalid_leading = ".eE+";

//         if (sv.size())
//         {
//             if (invalid_leading.contains(sv[0]))
//             {
//                 return false;
//             }

//             if (sv.size() > 1 && sv[0] == '0')
//             {
//                 return sv[1] == '.' || sv[1] == 'e' || sv[1] == 'E';
//             }
//         }

//         return true;
//     }

//     static number decode_json_number(Context& ctx)
//     {
//         assert(!ctx.eof());

//         size_t count = 0;
//         for (; count < ctx.size() && valid_number_character(ctx.peek(count)); ++count);

//         auto sv = ctx.to_string_view().substr(0, count);

//         if (!check_leading(sv))
//         {
//             throw std::runtime_error("Invalid leading character is not allowed in number.");
//         }

//         using SignedInteger = typename number::int_type;
//         using UnsignedInteger = typename number::uint_type;
//         using FloatingPoint = typename number::float_type;

//         // Try to parse as integer first, then unsigned integer, finally floating point.
//         if (auto result1 = from_chars_to_optional<SignedInteger>(sv); result1)
//         {
//             ctx.advance(count);
//             return number(*result1);
//         }
//         else if (auto result2 = from_chars_to_optional<UnsignedInteger>(sv); result2)
//         {
//             ctx.advance(count);
//             return number(*result2);
//         }
//         else if (auto result3 = from_chars_to_optional<FloatingPoint>(sv); result3)
//         {
//             ctx.advance(count);
//             return number(*result3);
//         }
//         else
//         {
//             throw std::runtime_error("Invalid number format.");
//         }
//     }

// public:

//     static number operator()(Context& ctx)
//     {
//         return decode_json_number(ctx);
//     }
// };

// template <typename Context>
// class decode2
// {
//     Context m_ctx;

//     value decode_null()
//     {
//         if (m_ctx.match("null", true))
//         {
//             return null();
//         }
//         throw std::runtime_error("Invalid literal, expected 'null'.");
//     }

//     value decode_boolean()
//     {
//         if (m_ctx.match("true", true))
//         {
//             return boolean(true);
//         }
//         else if (m_ctx.match("false", true))
//         {
//             return boolean(false);
//         }
//         throw std::runtime_error("Invalid literal, expected 'true' or 'false'.");
//     }

//     value decode_string()
//     {
//         return string_decoder<Context>()(m_ctx);
//     }
    
//     value decode_number()
//     {
//         return number_decoder<Context>()(m_ctx);
//     }

//     value decode_array()
//     {
//         m_ctx.match('[', true); // eat '['
//         m_ctx.skip_whitespace();

//         array arr;

//         if (m_ctx.match(']', true))
//         {
//             return arr;
//         }   
//         else 
//         {
//             while (1)
//             {
//                 auto val = decode_value();
//                 arr.emplace_back(std::move(val));
//                 m_ctx.skip_whitespace();

//                 if (m_ctx.match(']', true))
//                 {
//                     return arr;
//                 }

//                 if (!m_ctx.match(',', true))
//                 {
//                     throw std::runtime_error("Invalid array, expected ',' or ']'.");
//                 }

//                 m_ctx.skip_whitespace();
//             }
//         }
//     }

//     value decode_object()
//     {
//         if (!m_ctx.match('{', true))
//         {
//             throw std::runtime_error("Expected '{' at the beginning of object.");
//         }

//         m_ctx.skip_whitespace();

//         if (m_ctx.match('}', true))
//         {
//             return object();
//         }
//         else 
//         {
//             // parse key-value pair
//             object obj;

//             while (1) 
//             {
//                 // The inner loop may not ensure the next key-value pair
//                 // such as {"key" : 1, is illegal, so we must check the first character.
//                 if (!m_ctx.match('"', false))
//                 {
//                     throw std::runtime_error("Invalid object, expected string as key.");
//                 }

//                 auto key = decode_string();

//                 m_ctx.skip_whitespace();

//                 if (!m_ctx.match(':', true))
//                 {
//                     throw std::runtime_error("Invalid object, expected ':' after key.");
//                 }
                
//                 auto val = decode_value();
//                 obj.emplace(std::move(key.template as<string>()), std::move(val));
//                 m_ctx.skip_whitespace();

//                 if (m_ctx.match('}', true))
//                 {
//                     return obj;
//                 }

//                 if (!m_ctx.match(',', true))
//                 {
//                     throw std::runtime_error("Invalid object, expected ',' or '}'.");
//                 }

//                 m_ctx.skip_whitespace();
//             }
//         }
//     }
    
//     value decode_value()
//     {
//         m_ctx.skip_whitespace();

//         if (m_ctx.eof())
//         {
//             throw std::runtime_error("Unexpected end of input.");
//         }

//         switch (m_ctx.current())
//         {
//             case 'n': return decode_null();
//             case 't':
//             case 'f': return decode_boolean();
//             case '"': return decode_string();
//             case '[': return decode_array();
//             case '{': return decode_object();
//             default: return decode_number();
//         }
//     }

// public:

//     decode2(std::string_view sv) : m_ctx(sv) { }

//     value operator()()
//     {
//         m_ctx.skip_whitespace();
//         auto result = decode_value();
//         m_ctx.skip_whitespace();

//         if (!m_ctx.eof())
//         {
//             throw std::runtime_error("Trailing characters after JSON value.");
//         }
//         return result;
//     }

// };

// } // namespace detail

// namespace cpp::config::json
// {

// class decoder
// {

//     static bool valid_number_character(char ch)
//     {
//         struct valid_character_config
//         {
//             static int operator()(size_t x)
//             {
//                 [[assume(x < 256)]];  
//                 constexpr std::string_view sv = "-+.eE";
//                 return ::isdigit(x) || sv.contains(x); // x is less than 128 and it can convert to char.
//             }
//         };

//         static auto valid_characters = make_character_table(valid_character_config());
//         return valid_characters[ch];
//     }

//     static value make_error_code(error_code ec)
//     {
//         return ec;
//     }

//     value parse_array_or_object()
//     {
//         m_ctx.skip_whitespace();

//         switch (m_ctx.current())
//         {
//             case '{': return parse_object();
//             case '[': return parse_array();
//             default: return make_error_code(error_code::error_payload);
//         }
//     }

// public:

//     decoder(std::string_view context) : m_ctx(context) { }

//     value parse_null()
//     {
//         return m_ctx.match_literal_and_advance("null") 
//             ? null()
//             : make_error_code(error_code::illegal_literal); 
//     }

//     value parse_false()
//     {
//         return m_ctx.match_literal_and_advance("false") 
//             ? boolean(false) 
//             : make_error_code(error_code::illegal_literal); 
//     }

//     value parse_true()
//     {
//         return m_ctx.match_literal_and_advance("true") 
//             ? boolean(true) 
//             : make_error_code(error_code::illegal_literal); 
//     }

//     value parse_value()
//     {
//         if (m_ctx.eof())
//         {
//             return make_error_code(error_code::eof_error);
//         }

//         switch (m_ctx.current())
//         {
//             case 't': return parse_true();
//             case 'n': return parse_null();
//             case 'f': return parse_false();
//             case '[': return parse_array();
//             case '{': return parse_object();
//             case '"': return parse_string();
//             default: return parse_number();
//         }
//     }

//     value parse_array()
//     {
//         m_ctx.advance_unchecked(1); // eat '['
//         m_ctx.skip_whitespace();

//         array arr;

//         // FIXME: match -> current
//         if (m_ctx.current() == ']')
//         {
//             m_ctx.advance_unchecked(1); // eat ']'
//             return arr;
//         }   
//         else 
//         {
//             while (1)
//             {
//                 auto value = parse_value();

//                 if (!value)
//                 {
//                     return value; 
//                 }

//                 arr.emplace_back(std::move(value));
//                 m_ctx.skip_whitespace();

//                 if (m_ctx.current() == ']')
//                 {
//                     m_ctx.advance_unchecked(1); // eat ']'
//                     return arr;
//                 }

//                 if (!m_ctx.match_and_advance(','))
//                 {
//                     return make_error_code(error_code::illegal_array);
//                 }

//                 m_ctx.skip_whitespace();
//             }
//         }
//     }

//     value parse_object()
//     {
//         m_ctx.advance_unchecked(1); // eat '{'
//         m_ctx.skip_whitespace();

//         if (m_ctx.current() == '}')
//         {
//             m_ctx.advance_unchecked(1); // eat '}'
//             return object();
//         }
//         else if (m_ctx.current() == '\"')
//         {
//             // parse key-value pair
//             object obj;

//             while (1) 
//             {
//                 // The inner loop may not ensure the next key-value pair
//                 // such as {"key" : 1, is illegal, so we must check the first character.
//                 if (m_ctx.current() != '"')
//                 {
//                     return make_error_code(error_code::illegal_object);
//                 }

//                 auto key = parse_string();

//                 if (!key)
//                 {
//                     return key;
//                 }

//                 m_ctx.skip_whitespace();

//                 if (!m_ctx.match_and_advance(':'))
//                 {
//                     return make_error_code(error_code::illegal_object);
//                 }
                
//                 m_ctx.skip_whitespace();
//                 auto value = parse_value();

//                 if (!value)
//                 {
//                     return value;
//                 }

//                 obj.emplace(std::move(*key.as_ptr<string>()), std::move(value));
//                 m_ctx.skip_whitespace();

//                 if (m_ctx.current() == '}')
//                 {
//                     m_ctx.advance_unchecked(1); // eat '}'
//                     return obj;
//                 }

//                 if (!m_ctx.match_and_advance(','))
//                 {
//                     return make_error_code(error_code::illegal_object);
//                 }

//                 m_ctx.skip_whitespace();
//             }
//         }
//         else
//         {
//             return make_error_code(error_code::illegal_object);
//         }
//     }

//     value parse_string()
//     {
//         m_ctx.advance_unchecked(1); // eat '"'
//         string s;

//         while (1)
//         {
//             if (m_ctx.is_at_end())
//             {
//                 return make_error_code(error_code::illegal_string);
//             }

//             char ch = m_ctx.current();

//             if (ch == '"')
//             {
//                 m_ctx.advance_unchecked(1); // eat '"'
//                 return make_json<string>(std::move(s));
//             }

//             if (ch == '\\')
//             {
//                 m_ctx.advance_unchecked(1); // eat '\\'

//                 if (m_ctx.is_at_end())
//                 {
//                     return make_error_code(error_code::illegal_string);
//                 }

//                 switch (m_ctx.current())
//                 {
//                     case '"': s += '"'; break;    // quotation
//                     case '\\': s += '\\'; break;  // reverse solidus
//                     case '/': s += '/'; break;    // solidus
//                     case 'b': s += '\b'; break;   // backspace
//                     case 'f': s += '\f'; break;   // formfeed
//                     case 'n': s += '\n'; break;   // linefeed
//                     case 'r': s += '\r'; break;   // carriage return
//                     case 't': s += '\t'; break;   // horizontal tab
//                     case 'u': 
//                     {
//                         // https://codebrowser.dev/llvm/llvm/lib/Support/JSON.cpp.html#_ZN4llvm4json12_GLOBAL__N_110encodeUtf8EjRNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
//                         auto parse_4_hex = [this](std::string_view sv) -> optional<uint16_t> {
//                             if (sv.size() < 4 || !is_unicode<4>(sv.data()))
//                             {
//                                 return nullopt;
//                             }
//                             auto res = decode_unicode_from_char<4>(sv.data());  
//                             m_ctx.advance_unchecked(4);
//                             return res;
//                         };

//                         // Invalid UTF is not a JSON error (RFC 8529§8.2). It gets replaced by U+FFFD.
//                         auto invalid = [&] { s.append({'\xef', '\xbf', '\xbd'}); };
//                         uint16_t first;

//                         if (auto op = parse_4_hex(m_ctx.slice(1, 4)); !op)
//                         {
//                             return make_error_code(error_code::illegal_unicode);
//                         }
//                         else
//                         {
//                             first = *op;
//                         }

//                         while (1)
//                         {
//                             // basic multilingual plane, BMP(U+0000 - U+FFFF)
//                             if (first < 0xD800 || first >= 0xE000) [[likely]] 
//                             {
//                                 encode_unicode_to_utf8(std::back_inserter(s), first);
//                                 break;
//                             }
//                             if (first >= 0xDC00) [[unlikely]] 
//                             {
//                                 invalid();
//                                 break;
//                             }
//                             if (m_ctx.size() < 2 + 1 || m_ctx.peek(1) != '\\' || m_ctx.peek(2) != 'u') [[unlikely]] 
//                             {
//                                 invalid();
//                                 break;
//                             }
//                             m_ctx.advance_unchecked(2); // skip "\u"

//                             uint16_t second;

//                             if (auto op = parse_4_hex(m_ctx.slice(1, 4)); !op)
//                             {
//                                 return make_error_code(error_code::illegal_unicode);
//                             }
//                             else
//                             {
//                                 second = *op;
//                             }

//                             if (second < 0xDC || second >= 0xE000) [[unlikely]]
//                             {
//                                 invalid();
//                                 first = second;
//                                 continue;
//                             }
//                             uint32_t codepoint = 0x10000 | ((first - 0xD800) << 10) | (second - 0xDC00);
//                             encode_unicode_to_utf8(std::back_inserter(s), codepoint);
//                             break;
//                         }
//                         break;
//                     }   // 4 hex digits
//                     default: return make_error_code(error_code::illegal_string);
//                 }

//             }
//             else
//             {
//                 s += ch;
//             }

//             m_ctx.advance_unchecked(1);
//         }
//     }

//     value parse_number()
//     {
//         char ch = m_ctx.current();

//         // We use std::from_chars to help us parse number.
//         // This if-else branch is not necessary, but we want use it 
//         // to distinct the tow error cases.
//         if (ch == '-' || ::isdigit(ch))
//         {
//             auto startptr = m_ctx.data();

//             while (m_ctx.size() && valid_number_character(m_ctx.current()))
//             {
//                 m_ctx.advance_unchecked(1);
//             }

//             auto endptr = m_ctx.data();

//             // Since leading zeroes and 0x, 0b, 0o is not permitted, so if 
//             // a number started with 0, we assume it is a floating number.
//             if (startptr[0] != '0')
//             {
//                 // Try parse as integral first.
//                 if (auto value = from_chars_to_optional<number::int_type>(startptr, endptr); value)
//                 {
//                     return number(*value);
//                 }

//                 // Try parse as unsigned integral second.
//                 if (auto value = from_chars_to_optional<number::uint_type>(startptr, endptr); value)
//                 {
//                     return number(*value);
//                 }
//             }
//             else if (startptr + 1 == endptr) // "0"
//             {
//                 return number(0);
//             } 
//             else if (startptr[1] != '.') // Only "0." is allowed
//             {
//                 return make_error_code(error_code::illegal_number);
//             }

//             // Try parse as floating last.
//             if (auto value = from_chars_to_optional<number::float_type>(startptr, endptr); value)
//             {
//                 return number(*value);
//             }    

//             return make_error_code(error_code::illegal_number);
//         }
//         else
//         {
//             return make_error_code(error_code::unknown_character);
//         }
//     }

//     value parse()
//     {
//         return this->operator()();
//     }

//     value operator()()
//     {
//         // "A JSON payload should be an object or array."
//         // For debugging, we just parse value.
//         // auto root parse_array_or_object();
//         m_ctx.skip_whitespace();
//         auto root = parse_value();
        
//         if (!root)
//         {
//             return root;
//         }

//         m_ctx.skip_whitespace();
//         // return is_over() ? root : make_error_code(error_code::multi_value);
//         if (m_ctx.is_at_end())
//         {
//             return root;
//         }
//         else
//         {
//             return make_error_code(error_code::multi_value);
//         }
//     }

//     value operator()(std::string_view context)
//     {
//         m_ctx = context;
//         return this->operator()();
//     }

//     parse_context context() const
//     {
//         return m_ctx;
//     }

// private:

//     parse_context m_ctx;
// };

// inline value loads(std::string source)
// {
//     return decoder(source)();
// }

// inline value load(const char* filename)
// {
//     return loads(cpp::read_file_context(filename));
// }

// } // namespace cpp::config::json

// namespace cpp::config::json::literal
// {

// inline value operator""_json(const char* str, size_t len)
// {
//     return loads(std::string(str, len));
// }

// }

#include "decoder2.hpp"
