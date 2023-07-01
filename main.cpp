#include <leviathan/config_parser/common.hpp>
#include <leviathan/config_parser/value.hpp>
#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>
#include <bits/stdc++.h>

template <typename T>
struct use_pointer
{
    constexpr static bool value = sizeof(T) > 16;

    // using type = std::conditional_t<value, std::unique_ptr<T>, T>;
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

using leviathan::string::string_hash_key_equal;

class json_value;

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

using json_string = std::string;
using json_boolean = bool;
using json_null = std::nullptr_t;   // This may not suitable. The value of json_null is unique, the index in std::variant is enough to indicate it.
using json_array = std::vector<json_value>;
using json_object = std::unordered_map<json_string, json_value, string_hash_key_equal, string_hash_key_equal>;

using json_value_base = leviathan::config::value_base<
    std::variant, 
    to_unique_ptr, 
    json_null,
    json_boolean,
    json_number,
    json_string,
    json_array,
    json_object,
    char  // If some errors happen, return error_code.
>;


int main(int argc, char const* argv[])
{
    /* code */
    return 0;
}

