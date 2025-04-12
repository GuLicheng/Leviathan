#pragma once

#include "../common.hpp"
#include <leviathan/variable.hpp>
#include <cstdint>
#include <utility>

namespace cpp::config::json
{

struct store_self
{
    template <typename U>
    using type = U;

    template <typename T>
    static constexpr auto&& from_value(T&& t)
    {
        return (T&&)t;
    } 

    template <typename T>
    static constexpr auto to_address(T* t) 
    {
        return t;
    }
};

// std::variant<int64_t, uint64_t, double>
class number : public cpp::variable<store_self, int64_t, uint64_t, double>
{
    using base = variable<store_self, int64_t, uint64_t, double>;

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

    constexpr number() = delete;

    constexpr explicit number(std::signed_integral auto i) : base(int_type(i)) { }

    constexpr explicit number(std::floating_point auto f) : base(float_type(f)) { }

    constexpr explicit number(std::unsigned_integral auto u) : base(uint_type(u)) { }

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

    constexpr friend bool operator==(const number& x, const number& y) 
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

} // namespace cpp::config::json

