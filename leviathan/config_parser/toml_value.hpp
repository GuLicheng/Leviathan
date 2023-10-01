#pragma once

#include "value.hpp"

#include <stdexcept>
#include <format>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <utility>

namespace leviathan::config::toml
{
    // Some exceptions
    struct toml_parse_error : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct bad_toml_value_access : std::exception
    {
        const char* what() const noexcept override
        { return "bad_toml_value_access"; }
    };

    template <typename... Args>
    [[noreturn]] void throw_toml_parse_error(std::string_view fmt, Args&&... args)
    {
        auto msg = std::vformat(fmt, std::make_format_args((Args&&) args...));
        throw toml_parse_error(msg);
    }

    using to_pointer = to_unique_ptr_if_large_than<16>;

    class toml_value;

    namespace detail
    {
        template <typename T>
        class toml_array_base : public std::vector<T>
        {
            using base = std::vector<T>;

            bool m_locked = false;  // for fixed array or table array

        public:

            toml_array_base() = default;

            template <typename... Args>
            explicit toml_array_base(bool locked, Args&&... args) 
                : base((Args&&) args...), m_locked(locked) { }

            bool is_table_array() const 
            { return !m_locked; }

            bool is_array() const
            { return m_locked; }
        };

        template <typename K, typename V>
        class toml_table_base : public std::unordered_map<K, V, string_hash_key_equal, string_hash_key_equal>
        {
            using base = std::unordered_map<K, V, string_hash_key_equal, string_hash_key_equal>;

            bool m_locked = false;   // for table or inline table

            bool m_defined = false;  // for defining a super-table afterward 

        public:

            toml_table_base() = default;

            template <typename... Args>
            explicit toml_table_base(bool locked, Args&&... args) 
                : base((Args&&) args...), m_locked(locked) { }

            bool is_inline_table() const 
            { return m_locked; }

            bool is_table() const
            { return !m_locked; }
        
            bool is_defined() const
            { return m_defined; }

            void define_table()
            { m_defined = true; }
        };
    }

    using toml_boolean = bool;
    using toml_float = double;
    using toml_integer = int64_t;
    using toml_string = std::string;
    using toml_array = detail::toml_array_base<toml_value>;
    using toml_table = detail::toml_table_base<toml_string, toml_value>;

    // Can we use std::chrono directly?
    // YYYY-MM-DDTHH:MM:SS.XXXX+HH:MM
    // 1. offset date time: all
    // 2. local date time: local date + local time
    // 3. local date only
    // 4. local time only
    struct toml_datetime
    {
        enum class time_mode
        {
            offset_date_time,
            local_date_time,
            local_date,
            local_time,
        } m_type;

        // local date
        int m_year = 0;
        int m_month = 0;
        int m_day = 0;

        // local time
        int m_hour = 0;
        int m_minute = 0;
        int m_second = 0;
        int m_microsecond = 0;

        // offset 
        int m_hour_offset = 0;
        int m_minute_offset = 0;

        toml_string to_string() const
        {
            using enum toml_datetime::time_mode;
            switch (m_type)
            {
                case offset_date_time: return std::format("{}-{}-{}T{}:{}:{}.{}+-{}:{}", m_year, m_month, m_day, m_hour, m_minute, m_second, m_microsecond, m_hour_offset, m_minute_offset);
                case local_date_time: return std::format("{}-{}-{}T{}:{}:{}.{}", m_year, m_month, m_day, m_hour, m_minute, m_second, m_microsecond);
                case local_date: return std::format("{}-{}-{}", m_year, m_month, m_day);
                case local_time: return std::format("{}:{}:{}.{}", m_hour, m_minute, m_second, m_microsecond);
                default: std::unreachable();
            }
        }
    };

    using toml_value_base = value_base<
        to_pointer,
        toml_boolean,
        toml_integer,
        toml_float,
        toml_string,
        toml_array, 
        toml_table,
        toml_datetime
    >;

    class toml_value : public toml_value_base
    {
    public:

        toml_value() = delete;

        using toml_value_base::toml_value_base;
        using toml_value_base::operator=;

        auto& data() 
        { return m_data; }
        
        auto& data() const 
        { return m_data; }

        auto index() const  
        { return m_data.index(); }

        template <typename T>
        T& as()
        {
            auto ptr = as_ptr<T>();
            if (!ptr)
            {
                throw bad_toml_value_access();
            }
            return *ptr;
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

        template <typename T>
        const T* as_ptr() const
        { return const_cast<toml_value&>(*this).as_ptr<T>(); }

        toml_table& as_table() 
        { return as<toml_table>(); }
        
        toml_array& as_array() 
        { return as<toml_array>(); }
        
        toml_boolean& as_boolean()
        { return as<toml_boolean>(); }

        toml_integer& as_integer()
        { return as<toml_integer>(); }

        toml_string& as_string()
        { return as<toml_string>(); }

        toml_float& as_float()
        { return as<toml_float>(); }

        toml_datetime& as_date_time()
        { return as<toml_datetime>(); }

        // friend bool operator==(const toml_value& lhs, const toml_value& rhs) 
        // {
        //     return std::visit([]<typename T1, typename T2>(const T1& lhs, const T2& rhs) -> bool {
        //         if constexpr (std::same_as<T1, T2>)
        //         {
        //             if constexpr (is_pointer_v<T1>)
        //             {
        //                 return 
        //             }
        //             else    
        //             {

        //             }
        //         }
        //         else
        //         {
        //             return false;
        //         }
        //     }, lhs.data(), rhs.data());
        // }
    };

    toml_value make(std::integral auto num)
    { return toml_integer(num); }

    toml_value make(std::floating_point auto num)
    { return toml_float(num); }

    toml_value make(bool b)
    { return toml_boolean(b); }

    toml_value make(toml_array array)
    { return toml_value(std::move(array)); }

    toml_value make(toml_table table)
    { return toml_value(std::move(table)); }

    toml_value make(toml_string str)
    { return toml_value(std::move(str)); }

    toml_value make(toml_datetime datatime)
    { throw "NotImplement"; }

} // namespace leviathan::config::toml

