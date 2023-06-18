#pragma once

#include <leviathan/string/string_extend.hpp>
#include <leviathan/config_parser/item.hpp>
#include <leviathan/config_parser/common.hpp>
#include <leviathan/config_parser/encode.hpp>

#include <memory>
#include <variant>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <type_traits>

#include <assert.h>

namespace leviathan::config::json
{
    enum class error_code
    {
        // ok,
        eof_error,
        uninitialized,
        illegal_string,
        illegal_array,
        illegal_object,
        illegal_number,
        illegal_literal,
        illegal_boolean,
        illegal_root,
        illegal_unicode,
        unknown_character,
    };

    inline constexpr const char* error_infos[] = {
        // "ok",
        "end of file error",
        "uninitialized",
        "illegal_string",
        "illegal_array",
        "illegal_object",
        "illegal_number",
        "illegal_literal",
        "illegal_boolean",
        "illegal_root",
        "illegal_unicode",
        "unknown_character",
    };

    constexpr const char* report_error(error_code ec)
    {
        return error_infos[static_cast<int>(ec)];
    }

    using std::string_view;
    using std::string;
    using leviathan::string::arithmetic;
    using leviathan::string::whitespace_delimiters;
    using leviathan::string::string_hash_keyequal;
    using leviathan::string::string_viewable;

    class json_value;

    using json_number = double;
    using json_string = string;
    using json_boolean = bool;
    using json_null = std::nullptr_t;   // This may not suitable.
    using json_array = std::vector<json_value>;
    using json_object = std::unordered_map<json_string, json_value, string_hash_keyequal, string_hash_keyequal>;

    constexpr std::array<bool, 256> valid_unicode_character = [](){
        
        std::array<bool, 256> table;
        
        table.fill(false);

        for (int i = '0'; i <= '9'; ++i) table[i] = true;
        for (int i = 'a'; i <= 'f'; ++i) table[i] = true;
        for (int i = 'A'; i <= 'F'; ++i) table[i] = true;

        return table;
    }();

    bool is_unicode(string_view code)
    {
        if (code.size() != 4)
        {
            return false;
        }

        return valid_unicode_character[code[0]]
            && valid_unicode_character[code[1]]
            && valid_unicode_character[code[2]]
            && valid_unicode_character[code[3]];

    }

    template <typename T>
    struct to_unique_ptr : store_ptr<std::unique_ptr<T>, is_large_than_raw_pointer> { };

    using binder = config::bind<std::variant>::with<
        json_null,
        json_boolean,
        json_number,
        json_string,
        json_array,
        json_object,
        error_code  // If some errors happen, return error_code.
    >;

    using binder2 = binder::transform<to_unique_ptr>::type;

    template <typename T>
    concept json_value_able = binder2::contains<T>;

    class json_value
    {
    public:

        using value_type = binder2::type;

        value_type m_data;   

    public:

        json_value() : m_data(error_code::uninitialized) { }

        template <json_value_able T>
        json_value(T t) : m_data(std::move(t)) { }

        template <string_viewable... Svs>
        json_value& operator[](const Svs&... svs) 
        {
            string_view views[] = { string_view(svs)... };

            json_value* target = this;

            json_object default_object = json_object();

            for (auto sv : views)
            {
                auto obj = target->cast_unchecked<json_object>();
                assert(obj && "Json value shoule be json_object");
                auto it = obj->try_emplace(json_string(sv), std::make_unique<json_object>(json_object()));
                target = &(it.first->second);
            }

            return *target;
        }

        template <typename T>
        bool is() const
        {
            using U = typename to_unique_ptr<T>::type; 
            return std::holds_alternative<U>(m_data); 
        }

        bool is_number() const
        { return is<json_number>(); }
        
        bool is_boolean() const
        { return is<json_boolean>(); }

        bool is_null() const
        { return is<json_null>(); }

        bool is_array() const
        { return is<json_array>(); }

        bool is_object() const
        { return is<json_object>(); }

        bool is_string() const
        { return is<json_string>(); }

        explicit operator bool() const
        { return m_data.index() < std::variant_size_v<value_type> - 1; }

        template <typename T>
        T* cast_unchecked() 
        {
            if constexpr (!is_large_than_raw_pointer<T>::value)
                return std::get_if<T>(&m_data);
            else 
                return (*std::get_if<typename to_unique_ptr<T>::type>(&m_data)).get();
        }
    };

    json_value make(json_string str)
    { return std::make_unique<json_string>(std::move(str)); }

    json_value make(json_array arr)
    { return std::make_unique<json_array>(std::move(arr)); }

    json_value make(json_object obj)
    { return std::make_unique<json_object>(std::move(obj)); }

    json_value make(arithmetic auto num)
    { return json_number(num); }

    json_value make(json_boolean b)
    { return b; }

    json_value make(error_code ec)
    { return ec; }

    json_value make(const char* str)
    { return std::make_unique<json_string>(str); }  

    // https://www.json.org/json-en.html
    class parser
    {
        string m_context;

        string_view m_cur;

    public:

        parser(string context) 
            : m_context(std::move(context)), m_cur(m_context) { }
    
        json_value operator()() &&
        {
            // Json should started with array or object.
            return parse_array_or_object();
        }

    private:

        json_value parse_value()
        {
            skip_whitespace(); // necessary?

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

        json_value parse_array_or_object()
        {
            skip_whitespace();

            if (m_cur.empty())
            {
                return return_with_error_code(error_code::eof_error);
            }

            switch (current())
            {
                case '[': return parse_array();
                case '{': return parse_object();
                default: return return_with_error_code(error_code::illegal_root);
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
                return make(std::move(arr));
            }   
            else 
            {
                while (true)
                {
                    auto value = parse_value();

                    if (!value)
                    {
                        return return_with_error_code(error_code::illegal_array);
                    }

                    arr.emplace_back(std::move(value));

                    skip_whitespace();

                    if (current() == ']')
                    {
                        advance_unchecked(1); // eat ']'
                        return make(std::move(arr));
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
                return make(json_object());
            }
            else if (current() == '\"')
            {
                // parse key-value pair
                json_object obj;

                while (true) 
                {
                    auto key = parse_string();

                    if (!key)
                    {
                        return return_with_error_code(error_code::illegal_object);
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
                        return return_with_error_code(error_code::illegal_object);
                    }

                    obj.emplace(std::move(*key.cast_unchecked<json_string>()), std::move(value));

                    skip_whitespace();

                    if (current() == '}')
                    {
                        advance_unchecked(1); // eat '}'
                        return make(std::move(obj));
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

            while (true)
            {
                char ch = current();

                if (ch == '"')
                {
                    advance_unchecked(1); // eat '"'
                    return make(std::move(s));
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
                            if (!is_unicode(m_cur.substr(1, 4)))
                            {
                                return return_with_error_code(error_code::illegal_unicode);
                            }
                            unsigned unicode = decode_unicode_from_char(m_cur.data() + 1); // skip 'u'
                            encode_unicode_to_utf8(std::back_inserter(s), unicode);
                            advance_unchecked(4);
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

                if (double value; std::from_chars(startptr, endptr, value, std::chars_format::general).ec == std::errc())
                {
                    return value;
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
            auto idx = m_cur.find_first_not_of(whitespace_delimiters);
            m_cur.remove_prefix(idx == m_cur.npos ? m_cur.size() : idx);
        }

        static bool valid_number_character(char ch)
        {
            if (isdigit(ch))
            {
                return true;
            }
            switch (ch)
            {
                case '-':
                case '+':
                case '.':
                case 'e':
                case 'E': return true;
                default: return false;
            }
        }

        void advance_unchecked(size_t n)
        { m_cur.remove_prefix(n); }

        char current() const
        { return m_cur[0]; }

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

    namespace detail
    {
        template <typename OStream, typename Character>
        void json_padding(OStream& os, Character character, int count)
        {
            for (int i = 0; i < count; ++i)
                os << character;
        }
        inline constexpr const char* padding_character = " ";

        template <typename OStream>
        void json_serialize(OStream& os, const std::unique_ptr<json_array>& arr, int padding);

        template <typename OStream>
        void json_serialize(OStream& os, const json_boolean& boolean, int padding);

        template <typename OStream>
        void json_serialize(OStream& os, const json_number& number, int padding);

        template <typename OStream>
        void json_serialize(OStream& os, const std::unique_ptr<json_string>& string, int padding);

        template <typename OStream>
        void json_serialize(OStream& os, const std::unique_ptr<json_object>& object, int padding);

        template <typename OStream>
        void json_serialize(OStream& os, const json_null& null, int padding);

        template <typename OStream>
        void json_serialize(OStream& os, const json_value& arr, int padding);

        template <typename OStream>
        void json_serialize(OStream& os, const json_number& number, int padding)
        {
            json_padding(os, padding_character, padding);
            os << number;
        }

        template <typename OStream>
        void json_serialize(OStream& os, const std::unique_ptr<json_string>& stringptr, int padding)
        {
            auto& string = *stringptr;
            json_padding(os, padding_character, padding);
            os << '"' << string << '"';
        }

        template <typename OStream>
        void json_serialize(OStream& os, const std::unique_ptr<json_array>& arrayptr, int padding)
        {
            auto& array = *arrayptr;
            os << "[\n";
            for (std::size_t i = 0; i < array.size(); ++i)
            {
                if (i != 0) os << ", ";
                json_serialize(os, array[i], padding + 1);
            }
            os << '\n';
            json_padding(os, padding_character, padding);
            os << ']';
        }

        template <typename OStream>
        void json_serialize(OStream& os, const json_boolean& boolean, int padding)
        {
            json_padding(os, padding_character, padding);
            os << (boolean ? "true" : "false");
        }

        template <typename OStream>
        void json_serialize(OStream& os, const json_null&, int padding)
        {
            json_padding(os, padding_character, padding);
            os << "null";
        }

        template <typename OStream>
        void json_serialize(OStream& os, const std::unique_ptr<json_object>& objectptr, int padding)
        {
            auto& object = *objectptr;
            json_padding(os, padding_character, padding);
            os << "{\n";
            for (auto it = object.begin(); it != object.end(); ++it)
            {
                if (it != object.begin()) os << ",\n";
                json_padding(os, padding_character, padding);
                os << " \"";
                os << it->first;
                os << "\" : ";
                json_serialize(os, it->second, padding + 1);
            }
            os << '\n';
            json_padding(os, padding_character, padding);
            os << '}';
        }

        template <typename OStream>
        void json_serialize(OStream& os, const json_value& value, int padding)
        {
            std::visit([&os, padding]<typename T>(const T& t)
            {
                if constexpr (std::is_same_v<T, error_code>)
                    throw 0;
                json_serialize(os, t, padding);
            }, value.m_data);
        }

    } // namespace detail


} // namespace leviathan::config::json

