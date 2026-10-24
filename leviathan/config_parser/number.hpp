#pragma once

#include <cstdint>
#include <variant>
#include <concepts>
#include <format>

namespace cpp::config
{

class number
{
public:

    template <std::integral T>
    constexpr number(T value) : m_value(static_cast<int64_t>(value)) {}

    template <std::floating_point T>
    constexpr number(T value) : m_value(static_cast<double>(value)) {}

    constexpr number(const number&) = default;
    constexpr number& operator=(const number&) = default;

    template <std::integral T>
    constexpr number& operator=(T value)
    {
        m_value.emplace<int64_t>(value);
        return *this;
    }

    template <std::floating_point T>
    constexpr number& operator=(T value)
    {
        m_value.emplace<double>(value);
        return *this;
    }

    constexpr bool is_integral() const
    {
        return std::holds_alternative<int64_t>(m_value);
    }

    constexpr bool is_floating_point() const
    {
        return std::holds_alternative<double>(m_value);
    }

    constexpr int64_t as_integral() const
    {
        return is_integral() 
             ? std::get<int64_t>(m_value) 
             : static_cast<int64_t>(std::get<double>(m_value));
    }

    constexpr double as_floating_point() const
    {
        return is_floating_point()
             ? std::get<double>(m_value)
             : static_cast<double>(std::get<int64_t>(m_value)); 
    }

    constexpr size_t hash_code() const
    {
        return std::visit([](auto value) -> size_t {
            return std::hash<decltype(value)>{}(value);
        }, m_value);
    }


private:

    std::variant<int64_t, double> m_value;
};

}  // namespace cpp::config

template <>
struct std::hash<cpp::config::number>
{
    static constexpr auto operator()(const cpp::config::number& num)
    {
        return num.hash_code();
    }
};

// template <typename CharT>
// struct std::formatter<cpp::config::number, CharT>
// {
//     constexpr auto parse(auto& ctx) { return ctx.begin(); }

//     constexpr auto format(const cpp::config::number& num, auto& ctx)
//     {
//         if (num.is_integral())
//             return std::format_to(ctx.out(), "{}", num.as_integral());
//         else
//             return std::format_to(ctx.out(), "{}", num.as_floating_point());
//     }
// };






