#pragma once

#include "value.hpp"

#include <utility>
#include <memory>
#include <vector>
#include <iostream>
#include <compare>
#include <unordered_map>
#include <type_traits>

namespace leviathan::config::json
{
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
        error_payload,
        multi_value,
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
        "error_payload",
        "multi_value",
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
            signed_integer,
            unsigned_integer,
            floating,
        } m_type;
        
        union 
        {
            float_type m_f;
            int_type m_i;
            uint_type m_u;
        };

        template <typename T>
        T as() const
        {
            switch (m_type)
            {
                case number_type::floating: return static_cast<T>(m_f);
                case number_type::signed_integer: return static_cast<T>(m_i);
                case number_type::unsigned_integer: return static_cast<T>(m_u);
                default: std::unreachable();
            }
        }

    public:

        json_number() = delete;

        explicit json_number(std::signed_integral auto i) : m_i(i), m_type(number_type::signed_integer) { }

        explicit json_number(std::floating_point auto f) : m_f(f), m_type(number_type::floating) { }

        explicit json_number(std::unsigned_integral auto u) : m_u(u), m_type(number_type::unsigned_integer) { }

        bool is_signed_integer() const
        { return m_type == number_type::signed_integer; }

        bool is_unsigned_integer() const
        { return m_type == number_type::unsigned_integer; }

        bool is_integer() const
        { return m_type != number_type::floating; }

        bool is_floating() const
        { return m_type == number_type::floating; }

        float_type as_floating() const  
        { return static_cast<float_type>(*this); }

        uint_type as_unsigned_integer() const
        { return static_cast<uint_type>(*this); }

        int_type as_signed_integer() const
        { return static_cast<int_type>(*this); }

        explicit operator float_type() const
        { return as<float_type>(); }

        explicit operator int_type() const
        { return as<int_type>(); }

        explicit operator uint_type() const
        { return as<uint_type>(); }

        friend bool operator==(const json_number& x, const json_number& y) 
        {
            if (x.m_type != y.m_type)
            {
                return false;
            }
            
            using enum json_number::number_type;

            switch (x.m_type)
            {
                case floating: return std::abs(x.as_floating() - y.as_floating()) < 1e-5; // Is Ok?
                case signed_integer: return x.as_signed_integer() == y.as_signed_integer();
                case unsigned_integer: return x.as_unsigned_integer() == y.as_unsigned_integer();
            }
        }

        template <typename Char, typename Traits>
        friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const json_number& x)
        {
            if (x.is_floating())
                os << x.as_floating();
            else if (x.is_signed_integer())
                os << x.as_signed_integer();
            else
                os << x.as_unsigned_integer();
            return os;
        }
    };

    // Empty class maybe better. The value of json_null is unique, 
    // the index in std::variant is enough to indicate it.
    using json_null = std::nullptr_t;   

    using json_string = std::string;
    using json_boolean = bool;
    using json_array = std::vector<json_value>;
    using json_object = std::unordered_map<json_string, json_value, string_hash_key_equal, string_hash_key_equal>;

    // The std::shared_ptr may cause memory leak for cycling reference
    // and the raw pointer may free memory twice for cycling reference.
    using to_pointer = to_unique_ptr_if_large_than<16>;

    using json_value_base = value_base<
        to_pointer, 
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

        auto& data() 
        { return m_data; }

        auto& data() const
        { return m_data; }

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
            std::string_view views[] = { std::string_view(svs)... };

            json_value* target = this;

            json_object default_object = json_object();

            for (auto sv : views)
            {
                auto& obj = target->as<json_object>();
                auto it = obj.try_emplace(json_string(sv), json_object());
                target = &(it.first->second);
            }

            return *target;
        }

        template <typename T>
        T* as_ptr()
        {
            using U = typename mapped<T>::type;
            auto ptr = std::get_if<U>(&m_data);
            if constexpr (is_mapped<T>)
                return ptr ? std::to_address(*ptr) : nullptr;
            else
                return ptr;
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
                if constexpr (!is_mapped<T>)
                {
                    return *std::get_if<T>(&m_data);
                }
                else 
                {
                    using U = typename mapped<T>::type;
                    return *(*std::get_if<U>(&m_data)).get();
                }
            }
        }

        template <typename T, bool NoThrow = true>
        const T& as() const
        { return const_cast<json_value&>(*this).as<T>(); }

        bool is_integer() const
        {
            return is<json_number>() 
                && as<json_number>().is_integer();
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

} // namespace leviathan::config::json

