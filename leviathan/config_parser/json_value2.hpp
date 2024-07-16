#pragma once

#include <leviathan/value.hpp>
#include <leviathan/string/string_extend.hpp>

#include <utility>
#include <memory>
#include <vector>
#include <iostream>
#include <compare>
#include <format>
#include <unordered_map>
#include <type_traits>

namespace leviathan::config::json
{

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

using leviathan::string::string_viewable;
using leviathan::string::string_hash_key_equal;

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

class json_value;

// std::variant<int64_t, uint64_t, double>
class json_number : public leviathan::value<leviathan::store_self, int64_t, uint64_t, double>
{
    using base = leviathan::value<leviathan::store_self, int64_t, uint64_t, double>;

    template <typename T>
    constexpr T convert_to() const
    {
        return std::visit(
            [](const auto number) { return static_cast<T>(number); },
            m_data
        );
    }

public:

    using int_type = int64_t;
    using uint_type = uint64_t;
    using float_type = double;

    constexpr json_number() = delete;

    constexpr explicit json_number(std::signed_integral auto i) : base(int_type(i)) { }

    constexpr explicit json_number(std::floating_point auto f) : base(float_type(f)) { }

    constexpr explicit json_number(std::unsigned_integral auto u) : base(uint_type(u)) { }

    constexpr bool is_signed_integer() const
    {
        return std::holds_alternative<int_type>(m_data);
    }

    constexpr bool is_unsigned_integer() const
    {
        return std::holds_alternative<uint_type>(m_data);
    }

    constexpr bool is_integer() const
    {
        return !is_floating();
    }

    constexpr bool is_floating() const
    {
        return std::holds_alternative<float_type>(m_data);
    }

    constexpr float_type as_floating() const
    {
        return convert_to<float_type>();
    }

    constexpr uint_type as_unsigned_integer() const
    {
        return convert_to<uint_type>();
    }

    constexpr int_type as_signed_integer() const
    {
        return convert_to<int_type>();
    }

    constexpr explicit operator float_type() const
    {
        return as_floating();
    }

    constexpr explicit operator int_type() const
    {
        return as_signed_integer();
    }

    constexpr explicit operator uint_type() const
    {
        return as_unsigned_integer();
    }

    constexpr friend bool operator==(const json_number& x, const json_number& y) 
    {
        if (x.is_integer() && y.is_integer())
        {
            if (x.is_signed_integer() && y.is_signed_integer())
            {
                return std::cmp_equal(x.as_signed_integer(), y.as_signed_integer());
            }
            else if (x.is_unsigned_integer() && y.is_signed_integer())
            {
                return std::cmp_equal(x.as_unsigned_integer(), y.as_signed_integer());
            }
            else if (x.is_signed_integer() && y.is_unsigned_integer())
            {
                return std::cmp_equal(x.as_signed_integer(), y.as_unsigned_integer());
            }
            else
            {
                return std::cmp_equal(x.as_unsigned_integer(), y.as_unsigned_integer());
            }
        }
        else
        {
            constexpr double epsilon = 1e-5;
            return std::abs(x.as_floating() - y.as_floating()) < epsilon;
        }
    }
};

// Empty class maybe better. The value of json_null is unique, 
// the index in std::variant is enough to indicate it.
using json_null = std::nullptr_t;   

using json_string = std::string;
using json_boolean = bool;
using json_array = std::vector<json_value>;
using json_object = std::unordered_map<json_string, json_value, string_hash_key_equal, string_hash_key_equal>;

using json_value_base = leviathan::value<
    to_unique_ptr_if_large_than<16>, 
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
};

template <typename Object, typename... Args>
json_value make_json(Args&&... args)
{
    return Object((Args&&) args...);
}

}

namespace leviathan::json
{
using namespace ::leviathan::config::json;
}

namespace leviathan::config::json::detail
{

struct dump_helper
{
    std::string m_result;

    void operator()(const json_number& number, int count)
    {
        m_result += std::visit(
            [](const auto& x) { return std::format("{}", x); },
            number.data()
        );
    }

    void operator()(const typename json_value::actual<json_string>::type& strptr, int count) 
    {
        m_result += std::format("\"{}\"", *strptr);
    }

    void operator()(const std::unique_ptr<json_array>& arrptr, int count) 
    {
        auto& arr = *arrptr;
        m_result += '[';

        for (std::size_t i = 0; i < arr.size(); ++i)
        {
            if (i != 0) 
            {
                m_result.append(", ");
            }
            this->operator()(arr[i], 0);
        }

        m_result += ']';
    }

    void operator()(const json_boolean& boolean, int count) 
    {
        m_result.append((boolean ? "true" : "false"));
    }

    void operator()(const json_null&, int count) 
    {
        m_result.append("null"); 
    }

    void operator()(const typename json_value::actual<json_object>::type& objptr, int count) 
    {
        auto& object = *objptr;
        m_result += '{';
        for (auto it = object.begin(); it != object.end(); ++it)
        {
            if (it != object.begin()) 
            {
                m_result += ", ";
            }
            m_result += std::format(R"("{}" : )", it->first);
            this->operator()(it->second, count + 1);
        }
        m_result += '}';
    }

    void operator()(const error_code& ec, int count)
    {
        std::unreachable();
    }

    void operator()(const json_value& value, int count) 
    {
        std::visit([count, this]<typename T>(const T& t) {
            this->operator()(t, count);
        }, value.data());
    }
};

}

template <typename CharT>
struct std::formatter<leviathan::json::json_value, CharT> 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const leviathan::json::json_value& value, FmtContext& ctx) const
    {
        leviathan::json::detail::dump_helper dumper;
        dumper(value, 0);
        return std::ranges::copy(dumper.m_result, ctx.out()).out;
    }   
};


