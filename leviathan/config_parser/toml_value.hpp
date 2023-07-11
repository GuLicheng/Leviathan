#pragma once

#include "value.hpp"

#include <stdexcept>
#include <format>
#include <vector>
#include <unordered_map>
#include <algorithm>

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
    void throw_toml_parse_error(std::string_view fmt, Args&&... args)
    {
        auto msg = std::vformat(fmt, std::make_format_args((Args&&) args...));
        throw toml_parse_error(msg);
    }

    // Can we use std::chrono directly?
    // YYYY-MM-DDTHH:MM:SS.XXXX+HH:MM
    struct toml_data_time
    {
        int m_year = 0;
        int m_month = 0;
        int m_day = 0;

        int m_hour = 0;
        int m_minute = 0;
        int m_second = 0;
        int m_microsecond = 0;

        int m_hour_offset = 0;
        int m_minute_offset = 0;
    };

    struct to_pointer
    {
        template <typename T>
        constexpr auto operator()(T&& t) const
        {
            using U = std::remove_cvref_t<T>;
            if constexpr (sizeof(T) > 16)
            {
                return std::make_unique<U>((T&&)t);
            }
            else
            {
                return t;
            }
        }
    };

    class toml_value;

    namespace detail
    {
        template <typename T, typename Allocator = std::allocator<T>>
        class toml_array_base : public std::vector<T, Allocator>
        {
            using base = std::vector<T, Allocator>;

            bool m_locked = false;

        public:

            toml_array_base() = default;

            template <typename... Args>
            explicit toml_array_base(bool locked, Args&&... args) 
                : base((Args&&) args...), m_locked(locked) { }

            bool is_table_array() const 
            { return !m_locked; }

            bool is_array() const
            { return m_locked; }

            bool is_all_same_type() const
            {
                if (this->empty())
                {
                    return true;
                }
                const auto idx = this->front().data().index();
                return std::ranges::all_of(*this, [=](const auto& x) {
                    return x.data().index() == idx;
                });
            }

        };
    }

    /**
     * How to distinct the array and table array?
     *  - Use two type : toml_array(std::vector<toml_value>) and toml_table_array(std::vector<toml_table>).
     *  - Add another flag for toml_array to check whether it is a toml_table_array.
     *  - Record all table_array when parsing.
    */
    using toml_array = detail::toml_array_base<toml_value>;

    using toml_boolean = bool;
    using toml_float = double;
    using toml_integer = int64_t;
    using toml_string = std::string;
    using toml_table = std::unordered_map<toml_string, toml_value, string_hash_key_equal, string_hash_key_equal>;

    using toml_value_base = value_base<
        to_pointer,
        toml_boolean,
        toml_integer,
        toml_float,
        toml_string,
        toml_array, 
        toml_table,
        toml_data_time
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

        toml_data_time& as_data_time()
        { return as<toml_data_time>(); }
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

    toml_value make(toml_data_time datatime)
    { throw "NotImplement"; }

} // namespace leviathan::config::toml

