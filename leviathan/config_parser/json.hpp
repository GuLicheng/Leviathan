// https://www.json.org/json-en.html
#pragma once

#include <leviathan/string/string_extend.hpp>
#include <leviathan/config_parser/value.hpp>
#include <leviathan/config_parser/common.hpp>

#include <utility>
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
    using std::string_view;
    using std::string;
    using leviathan::string::arithmetic;
    // using leviathan::string::whitespace_delimiters;
    using leviathan::string::string_hash_key_equal;
    using leviathan::string::string_viewable;

    enum class error_code
    {
        ok,
        eof_error,
        uninitialized,
        illegal_string,
        illegal_array,
        illegal_object,
        illegal_number,
        illegal_literal,
        illegal_boolean,
        illegal_unicode,
        unknown_character,
    };

    inline constexpr const char* error_infos[] = {
        "ok",
        "end of file error",
        "uninitialized",
        "illegal_string",
        "illegal_array",
        "illegal_object",
        "illegal_number",
        "illegal_literal",
        "illegal_boolean",
        "illegal_unicode",
        "unknown_character",
    };

    constexpr const char* report_error(error_code ec)
    {
        return error_infos[static_cast<int>(ec)];
    }

    struct bad_json_value_access : std::exception
    {
        const char* what() const noexcept override
        { return "bad_json_value_access"; }
    };

    namespace detail
    {
        constexpr bool is_unicode(string_view code)
        {
            if (code.size() != 4)
            {
                return false;
            }
            return isxdigit(code[0])
                && isxdigit(code[1])
                && isxdigit(code[2])
                && isxdigit(code[3]);
        }
    }

    class json_value;

    // std::variant<int64_t, uint64_t, double>
    class json_number
    {
    public:

        using int_type = int64_t;
        using uint_type = uint64_t;
        using float_type = double;

    private:

        enum struct number_type 
        {
            SignedIntegral,
            UnsignedIntegral,
            FloatingPoint,
        };

        union 
        {
            float_type m_f;
            int_type m_i;
            uint_type m_u;
        };

        number_type m_type;

        template <typename T>
        T as() const
        {
            switch (m_type)
            {
                case number_type::FloatingPoint: return static_cast<T>(m_f);
                case number_type::SignedIntegral: return static_cast<T>(m_i);
                case number_type::UnsignedIntegral: return static_cast<T>(m_u);
                default: std::unreachable();
            }
        }

    public:

        json_number() = delete;

        explicit json_number(std::signed_integral auto i) : m_i(i), m_type(number_type::SignedIntegral) { }

        explicit json_number(std::floating_point auto f) : m_f(f), m_type(number_type::FloatingPoint) { }

        explicit json_number(std::unsigned_integral auto u) : m_u(u), m_type(number_type::UnsignedIntegral) { }

        bool is_signed_integral() const
        { return m_type == number_type::SignedIntegral; }

        bool is_unsigned_integral() const
        { return m_type == number_type::UnsignedIntegral; }

        bool is_integral() const
        { return m_type != number_type::FloatingPoint; }

        bool is_floating() const
        { return m_type == number_type::FloatingPoint; }

        float_type as_floating() const  
        { return static_cast<float_type>(*this); }

        uint_type as_unsigned_integral() const
        { return static_cast<uint_type>(*this); }

        int_type as_signed_integral() const
        { return static_cast<int_type>(*this); }

        explicit operator float_type() const
        { return as<float_type>(); }

        explicit operator int_type() const
        { return as<int_type>(); }

        explicit operator uint_type() const
        { return as<uint_type>(); }

        template <typename Char, typename Traits>
        friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const json_number& x)
        {
            if (x.is_floating())
                os << x.as_floating();
            else if (x.is_signed_integral())
                os << x.as_signed_integral();
            else
                os << x.as_unsigned_integral();
            return os;
        }
    };

    using json_string = string;
    using json_boolean = bool;
    using json_null = std::nullptr_t;   // This may not suitable. The value of json_null is unique, the index in std::variant is enough to indicate it.
    using json_array = std::vector<json_value>;
    using json_object = std::unordered_map<json_string, json_value, string_hash_key_equal, string_hash_key_equal>;

    template <typename T>
    struct use_pointer 
    {
        constexpr static bool value = sizeof(T) > 16;

        using type = std::conditional_t<value, std::unique_ptr<T>, T>;
    };

    struct to_unique_ptr
    {
        template <typename T>
        constexpr auto operator()(T&& t) const
        {
            using U = std::remove_cvref_t<T>;
            if constexpr (use_pointer<U>::value)
            {
                return std::make_unique<U>((T&&)t);
            }
            else
            {
                return t;
            }
        }
    };

    static_assert(!use_pointer<json_null>::value);
    static_assert(!use_pointer<json_boolean>::value);
    static_assert(!use_pointer<json_number>::value);
    static_assert(!use_pointer<error_code>::value);

    static_assert(use_pointer<json_string>::value);
    static_assert(use_pointer<json_array>::value);
    static_assert(use_pointer<json_object>::value);

    using json_value_base = value_base<
        std::variant, 
        to_unique_ptr, 
        json_null,
        json_boolean,
        json_number,
        json_string,
        json_array,
        json_object,
        error_code  // If some errors happen, return error_code.
    >;

    // Clang will complain incomplete type but GCC and MSVC are OK.
    class json_value : public json_value_base
    {
    public:

        using typename json_value_base::value_type;

        template <typename T>
        bool is() const
        {
            using U = typename use_pointer<T>::type;
            return std::holds_alternative<U>(m_data); 
        }

        template <typename T, bool NoThrow = true>
        T& as() 
        {
            if constexpr (!NoThrow)
            {
                if (!is<T>())
                    throw bad_json_value_access();
                return as<T, NoThrow>();
            }
            else
            {
                if constexpr (!use_pointer<T>::value)
                {
                    return *std::get_if<T>(&m_data);
                }
                else 
                {
                    using U = typename use_pointer<T>::type;
                    return *(*std::get_if<U>(&m_data)).get();
                }
            }
        }

        template <typename T, bool NoThrow = true>
        const T& as() const
        { return const_cast<json_value&>(*this).as<T>(); }

        template <typename T>
        optional<T&> try_as()
        {
            if (!is<T>())
            {
                return nullopt;
            }
            return as<T>();
        }

    public:

        json_value() : json_value_base(error_code::uninitialized) { }

        using json_value_base::json_value_base;
        using json_value_base::operator=;

        template <string_viewable... Svs>
        json_value& operator[](const Svs&... svs) 
        {
            string_view views[] = { string_view(svs)... };

            json_value* target = this;

            json_object default_object = json_object();

            for (auto sv : views)
            {
                auto& obj = target->cast_unchecked<json_object>();
                auto it = obj.try_emplace(json_string(sv), json_object());
                target = &(it.first->second);
            }

            return *target;
        }

        bool is_integral() const
        {
            return is<json_number>() 
                && as<json_number>().is_integral();
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

        error_code ec() const
        { 
            auto code = std::get_if<error_code>(&m_data);
            return code ? *code : error_code::ok;
        }

        optional<json_number&> as_number()
        { return try_as<json_number>(); }

        optional<json_boolean&> as_boolean()
        { return try_as<json_boolean>(); }

        optional<json_null&> as_null() 
        { return try_as<json_null>(); }

        optional<json_array&> as_array()
        { return try_as<json_array>(); }

        optional<json_object&> as_object()
        { return try_as<json_object>(); }

        optional<json_string&> as_string()
        { return try_as<json_string>(); }

        template <typename T>
        T& cast_unchecked() 
        {
            return as<T>();
        }
    };

    json_value make(json_null)
    { return json_null(); }

    json_value make(json_string str)
    { return str; }

    json_value make(json_array arr)
    { return arr; }

    json_value make(json_object obj)
    { return obj; }

    json_value make(arithmetic auto num)
    { return json_number(num); }

    json_value make(json_boolean b)
    { return b; }

    json_value make(error_code ec)
    { return ec; }

    json_value make(const char* str)
    { return json_string(str); }  

    class parser
    {
        string m_context;
        string_view m_cur;

    public:

        parser(string context) 
            : m_context(std::move(context)), m_cur(m_context) { }
    
        json_value operator()() &&
        {
            return parse_value();
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
                while (true)
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

                while (true) 
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

                    obj.emplace(std::move(key.cast_unchecked<json_string>()), std::move(value));

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

            while (true)
            {
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
                                if (!detail::is_unicode(sv))
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

                            while (true)
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

                // Try parse as integral first.
                if (auto value = from_chars_to_optional<json_number::int_type>(startptr, endptr); value)
                {
                    return json_number(*value);
                }

                // TODO: try parse as unsigned integral second.

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
                    [[assume(i < 128)]];
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
                    [[assume(x < 128)]];  
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
            auto& arr = *arrayptr;
            os << "[\n";
            for (std::size_t i = 0; i < arr.size(); ++i)
            {
                if (i != 0) os << ", ";
                json_serialize(os, arr[i], padding + 1);
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
                {
                    std::unreachable();
                }
                json_serialize(os, t, padding);
            }, value.m_data);
        }

    } // namespace detail


} // namespace leviathan::config::json

