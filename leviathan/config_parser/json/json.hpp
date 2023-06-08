#ifndef JSON_HPP
#define JSON_HPP

#include "../encode.hpp"

#include <iterator>
#include <fstream>
#include <variant>
#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <algorithm>
#include <type_traits>
#include <assert.h>

namespace leviathan::json
{

    class json_error : public std::exception
    {
    public:
        json_error(const char* msg = "Unknown Error") : m_msg{ msg } { }

        constexpr const char* what() const noexcept override
        {
            return m_msg;
        }
    private:
        const char* m_msg;
    };

    enum class json_error_code
    {
        // TODO: replace exception with error code
        ok = 0,
        eof_error,
        uninitialized,
        illegal_string,
        illegal_array,
        illegal_object,
        illegal_number,
        illegal_literal,
        illegal_boolean
    };

    constexpr const char* json_error_info[]
    {
        "OK",
        "End of file error",
        "uninitialized",
        "illegal_string",
        "illegal_array",
        "illegal_object",
        "illegal_number",
        "illegal_literal",
        "illegal_boolean"    
    };

    constexpr const char* report_error(json_error_code ec)
    {
        return json_error_info[static_cast<int>(ec)];
    }

    // string、number、object、array、(true、false)、null

    // declaration 
    struct json_boolean;
    struct json_array;
    struct json_number;
    struct json_string;
    struct json_null;
    struct json_object;
    struct json_value;
    struct json_entry;

    struct json_boolean
    {
        bool m_val;
    };
    struct json_array
    {
        std::vector<json_value> m_val;

        template <typename JsonValue>
        void append(JsonValue&& value)
        {
            m_val.emplace_back(std::forward<JsonValue>(value));
        }

    };
    struct json_number
    {
        double m_val;
    };
    struct json_string
    {
        std::string m_val;
    };
    struct json_null
    {
        template <typename OStream>
        void serialization(OStream& os)
        {
            os << "null";
        }
    };
    struct json_object
    {
        std::vector<json_entry> m_val;

        template <typename String, typename JsonValue>
        void append(String&& string, JsonValue&& value)
        {
            m_val.emplace_back(json_entry{ .m_key = std::forward<String>(string), .m_val = std::forward<JsonValue>(value) });
        }

    };

    namespace detail
    {
        // see leviathan::meta::should_be
        template <typename T, typename... Ts>
        struct contains : std::disjunction<std::is_same<std::remove_cvref_t<T>, Ts>...> { };

        template <typename T, typename... Ts>
        constexpr bool contains_v = contains<T, Ts...>::value;

        static_assert(contains_v<int&, int, double>);
        static_assert(contains_v<int const&, int, double>);
        static_assert(contains_v<int&&, int, double>);
        static_assert(!contains_v<int, char, double>);
    }

    struct json_value
    {
        std::variant<
            json_null,
            json_boolean,
            json_array,
            json_number,
            json_object,
            json_string> m_val;

        template <typename T> 
        requires (detail::contains_v<T, json_null, json_boolean, json_array, json_number, json_object, json_string>)
        json_value(T&& t) : m_val{ std::forward<T>(t) } { }

        template <typename T> 
        requires (detail::contains_v<T, json_null, json_boolean, json_array, json_number, json_object, json_string>)
        json_value& operator=(T&& t)
        {
            m_val = std::forward<T>(t);
            return *this;
        }

        json_value() = default;
        json_value(const json_value& rhs) = default;
        json_value(json_value&& rhs) = default;
        json_value& operator=(const json_value& rhs) = default;
        json_value& operator=(json_value&& rhs) = default;

        template <typename T>
        T* cast()
        {
            return std::get_if<T>(&m_val);
        }

        template <typename JsonValue>
        bool try_append_value(JsonValue&& value)
        {
            json_array* ls = cast<json_array>();
            if (!ls)
                return false;
            ls->m_val.emplace_back(std::forward<JsonValue>(value));
            return true;
        }
        
        template <typename String, typename JsonValue>
        bool try_append_entry(String&& key, JsonValue&& value)
        {
            json_object* obj = cast<json_object>();
            if (!obj)
                return false;
            obj->m_val.emplace_back(json_entry{ .m_key = std::forward<String>(key), .m_val = std::forward<JsonValue>(value) });
            return true;
        }

        template <typename T>
        bool try_assign(T&& t)
        {
            using U = std::remove_cvref_t<T>;
            auto self = cast<U>();
            if (!self)
                return false;   
            (*self) = std::forward<T>(t);
            return true;
        }

    };
    struct json_entry
    {
        std::string m_key;
        json_value m_val;
    };

    template <typename OStream, typename Character>
    void json_padding(OStream& os, Character character, int count)
    {
        for (int i = 0; i < count; ++i)
            os << character;
    }

    constexpr static auto padding_character = " ";

    template <typename OStream>
    void json_serialize(OStream& os, const json_array& arr, int padding);

    template <typename OStream>
    void json_serialize(OStream& os, const json_boolean& boolean, int padding);

    template <typename OStream>
    void json_serialize(OStream& os, const json_number& number, int padding);

    template <typename OStream>
    void json_serialize(OStream& os, const json_string& string, int padding);

    template <typename OStream>
    void json_serialize(OStream& os, const json_object& object, int padding);

    template <typename OStream>
    void json_serialize(OStream& os, const json_null& null, int padding);

    template <typename OStream>
    void json_serialize(OStream& os, const json_value& arr, int padding);

    template <typename OStream>
    void json_serialize(OStream& os, const json_entry& entry, int padding);

    template <typename OStream>
    void json_serialize(OStream& os, const json_number& number, int padding)
    {
        json_padding(os, padding_character, padding);
        os << number.m_val;
    }

    template <typename OStream>
    void json_serialize(OStream& os, const json_string& string, int padding)
    {
        json_padding(os, padding_character, padding);
        os << '"' << string.m_val << '"';
    }

    template <typename OStream>
    void json_serialize(OStream& os, const json_array& array, int padding)
    {
        os << "[\n";
        for (std::size_t i = 0; i < array.m_val.size(); ++i)
        {
            if (i != 0) os << ", ";
            json_serialize(os, array.m_val[i], padding + 1);
        }
        os << '\n';
        json_padding(os, padding_character, padding);
        os << ']';
    }

    template <typename OStream>
    void json_serialize(OStream& os, const json_boolean& boolean, int padding)
    {
        json_padding(os, padding_character, padding);
        os << (boolean.m_val ? "true" : "false");
    }

    template <typename OStream>
    void json_serialize(OStream& os, const json_null&, int padding)
    {
        json_padding(os, padding_character, padding);
        os << "null";
    }

    template <typename OStream>
    void json_serialize(OStream& os, const json_object& object, int padding)
    {
        json_padding(os, padding_character, padding);
        os << "{\n";
        for (std::size_t i = 0; i < object.m_val.size(); ++i)
        {
            if (i != 0) os << ",\n";
            json_serialize(os, object.m_val[i], padding + 1);
        }
        os << '\n';
        json_padding(os, padding_character, padding);
        os << '}';
    }

    template <typename OStream>
    void json_serialize(OStream& os, const json_entry& entry, int padding)
    {
        json_padding(os, padding_character, padding);
        os << " \"";
        os << entry.m_key;
        os << "\" : ";
        json_serialize(os, entry.m_val, padding + 1);
    }

    template <typename OStream>
    void json_serialize(OStream& os, const json_value& value, int padding)
    {
        std::visit([&os, padding]<typename T>(const T& t)
        {
            json_serialize(os, t, padding);
        }, value.m_val);
    }

    class json_reader
    {
    public:

        json_reader(std::string context) : m_buf{ std::move(context) } { }

        template <typename OStream = std::basic_ostream<char>>
        void serialize(OStream& os = std::cout) const
        {
            json_serialize(os, m_val, 0);
        }

        const json_value& root() const
        {
            return m_val;
        }

        json_value& root()
        {
            return m_val;
        }

        template <typename T>
        T* cast()
        {
            return m_val.cast<T>();
        }

        void parse()
        {
            m_err = json_error_code::ok;
            m_cur = m_buf.data();
            m_sentinel = m_cur + m_buf.size();
            m_val = parse_value();  // root should be json_object or json_array
        }
        
        explicit operator bool() const
        {
            return m_err == json_error_code::ok;
        }

        auto error_code() const 
        {
            return m_err;
        }

    private:
        std::string m_buf;
        json_error_code m_err = json_error_code::uninitialized;
        char* m_cur;
        char* m_sentinel;
        int m_line = 0;
        json_value m_val; // root

        /**
        * Skip blank charactor
        */
        void skip_blank()
        {
            while (m_cur != m_sentinel && is_blank(*m_cur))
                ++m_cur;
        }

        /**
        * Check whether char `c` is a blank
        */
        bool is_blank(int c)
        {
            if (c == '\n')
            {
                m_line++;  
                return true; 
            }
            return c == ' ' || c == '\r' || c == '\t';
        }

        json_value parse_value()
        {
            skip_blank();
            if (m_cur == m_sentinel)
            {
                //throw json_error{ "Expected parse value but end..." };
                return exit_parse(json_error_code::eof_error);
            }
            json_value value;
            switch (*m_cur)
            {
            case ' ':
            case '\n':
            case '\t':
            case '\r': skip_blank(); parse_value(); break;
            case '"': value = parse_string(); break;
            case '{': value = parse_object(); break;
            case '[': value = parse_array(); break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-':
            case '+': value = parse_number(); break;
            default: value = parse_literal(); break;
            }
            return value;
        }

        /**
        *   lazy parser, just get number string
        */
        json_string parse_number()
        {
            auto begin = m_cur;
            while (m_cur != m_sentinel && (::isdigit(*m_cur) || *m_cur == '.' || *m_cur == 'e') || *m_cur == '+' || *m_cur == '-')
                m_cur++;
            // value.m_val = std::stod(std::string{ begin, m_cur });
            return {
                .m_val = std::string{ begin, m_cur }
            };
        }

        json_array parse_array()
        {
            json_array arr;
            ++m_cur; // eat '['
            while (m_cur != m_sentinel)
            {
                skip_blank();
                switch (*m_cur)
                {
                case ']': ++m_cur; return arr; // OK
                case ',': ++m_cur; skip_blank(); break;
                default:
                    json_value val = parse_value();
                    arr.m_val.emplace_back(std::move(val));
                    break;
                }
            }
            return arr;
        }

        json_value parse_literal()
        {
            auto ch = *m_cur;
            switch (*m_cur)
            {
            case 't': return parse_true();
            case 'f': return parse_false();
            case 'n': return parse_null();
            default: return exit_parse(json_error_code::illegal_literal);
            }
            // throw json_error{ "Not legal literal " };
        }

        json_value parse_true()
        {
            if (m_sentinel - m_cur < 4)
            {
                return exit_parse(json_error_code::illegal_literal);
            }
            if (m_cur[1] == 'r' && m_cur[2] == 'u' && m_cur[3] == 'e')
            {
                m_cur += 4;
                return json_boolean{ .m_val = true };
            }
            return exit_parse(json_error_code::illegal_literal);
        }

        template <typename Return = json_value>
        Return exit_parse(json_error_code ec)
        {
            m_err = ec;
            m_cur = m_sentinel;
            return { };
        }

        json_value parse_null()
        {
            if (m_sentinel - m_cur < 4)
            {
                return exit_parse(json_error_code::illegal_literal);
            }
            if (m_cur[1] == 'u' && m_cur[2] == 'l' && m_cur[3] == 'l')
            {
                m_cur += 4;
                return json_null{ };
            }
            return exit_parse(json_error_code::illegal_literal);
        }

        json_value parse_false()
        {
            if (m_sentinel - m_cur < 5)
            {
                return exit_parse(json_error_code::illegal_literal);
            }
            if (m_cur[1] == 'a' && m_cur[2] == 'l' && m_cur[3] == 's' && m_cur[4] == 'e')
            {
                m_cur += 5;
                return json_boolean{ .m_val = false };
            }
            return exit_parse(json_error_code::illegal_literal);
        }

        json_object parse_object()
        {
            // TODO:
            json_object obj;
            ++m_cur;  // eat '{'
            skip_blank();
            while (m_cur != m_sentinel && *m_cur != '}')
            {
                // { "KeyString1 " : Value , "KeyString2" : Value... }
                //   ^                     ^                         ^
                skip_blank();
                switch (*m_cur)
                {
                case '"': obj.m_val.emplace_back(parse_entry()); break; // OK
                case ',': ++m_cur; // [[fallthrough]];
                case '}': break;
                default: return exit_parse<json_object>(json_error_code::illegal_object);
                }
            }
            ++m_cur; // eat '}'
            return obj;
        }

        json_string parse_string()
        {
            // "KeyString2"  "Stirng\"\"" -> String""
            m_cur++;  // eat first '"'
            auto old = m_cur;
            std::string context;
            for (;m_cur != m_sentinel && *m_cur != '"'; m_cur++)
            {
                if (*m_cur == '\\')
                {
                    m_cur++;
                    if (m_cur == m_sentinel)
                    {
                        return exit_parse<json_string>(json_error_code::illegal_string);
                        //throw json_error{ "Expected 'turbfvu' but got another charactor" };
                    }
                    switch (*m_cur)
                    {
                        case 'n': context += '\n'; break;
                        case 't': context += '\t'; break;
                        case 'r': context += '\r'; break;
                        case 'b': context += '\b'; break;
                        case 'f': context += '\f'; break;
                        case 'v': context += '\v'; break;
                        case '"': context += '"'; break;
                        case '\\': context += '\\'; break;
                        case 'u': {
                            m_cur++;
                            unsigned unicode = decode_unicode_from_char(m_cur);
                            encode_unicode_to_utf8(std::back_inserter(context), unicode);
                            m_cur += 3;
                            break;
                        } // ...
                        default: context += *m_cur; break;
                    }
                }
                else
                    context += *m_cur;
            }
            m_cur++; // eat last '"'
            return json_string{ .m_val = std::move(context) };
        }

        json_entry parse_entry()
        {
            // { "KeyString1 " : Value , "KeyString2" : Value... }
            json_string key = parse_string();
            skip_blank();
            if (*m_cur != ':')
            {
                // std::cout << (int)*m_cur << ' ';
                // throw json_error{ "Expected ':' but got another charactor" }; // match ':'
                return exit_parse<json_entry>(json_error_code::illegal_object);
            }
            ++m_cur;
            skip_blank();
            json_value val = parse_value();
            return json_entry{ .m_key = std::move(key.m_val), .m_val = std::move(val) };
        }

    };


    struct json_entry_recursive_iterator
    {

        bool operator==(std::default_sentinel_t) const
        {
            return is_over();
        }

        bool operator!=(std::default_sentinel_t) const
        {
            return !this->operator==(std::default_sentinel);
        }

        bool is_over() const
        {
            struct NotImpl { };
            throw NotImpl{ };
        }

        json_value* m_root;
    };



} // end of leviathan

#endif