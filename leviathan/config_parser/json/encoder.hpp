#pragma once

#include <leviathan/config_parser/formatter.hpp>
#include <leviathan/config_parser/json/value.hpp>
#include <leviathan/extc++/concepts.hpp>
#include <leviathan/type_caster.hpp>

namespace cpp::config::json
{
    
namespace detail
{

[[deprecated("Use encoder2 instead")]]
struct encoder
{
    std::string m_result;

    void operator()(const number& number, int count)
    {
        m_result += std::visit(
            [](const auto& x) { return std::format("{}", x); },
            number.data()
        );
    }

    void operator()(const string& str, int count) 
    {
        m_result += std::format("\"{}\"", str);
    }

    void operator()(const array& arr, int count) 
    {
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

    void operator()(const boolean& boolean, int count) 
    {
        m_result.append((boolean ? "true" : "false"));
    }

    void operator()(const null&, int count) 
    {
        m_result.append("null"); 
    }

    void operator()(const object& object, int count) 
    {
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

    void operator()(const value& value, int count) 
    {
        std::visit([count, this]<typename T>(const T& x) {
            this->operator()(value::accessor()(x), count);
        }, value.data());
    }
};

struct encoder2
{
    static std::string operator()(const number& num)
    {
        return std::visit(
            [](const auto& x) { return std::format("{}", x); },
            num.data()
        );
    }

    static std::string operator()(const string& str) 
    {
        return std::format("\"{}\"", str);
    }

    static std::string operator()(const array& arr) 
    {
        return format_sequence(encoder2(), arr, ",", "[{}]");
    }

    static std::string operator()(const boolean& boolean) 
    {
        return boolean ? "true" : "false";
    }

    static std::string operator()(const null&) 
    {
        return "null";
    }

    static std::string operator()(const object& obj) 
    {
        return format_map(encoder2(), obj);
    }

    [[noreturn]] static std::string operator()(const error_code& ec)
    {
        std::unreachable();
    }

    static std::string operator()(const value& v) 
    {
        return std::visit([]<typename T>(const T& x) {
            return encoder2::operator()(value::accessor()(x));
        }, v.data());
    }
};

template <typename T> 
struct caster;

template <>
struct caster<std::string>
{
    static std::string operator()(const value& v)
    {
        return encoder2()(v);
    }
};

template <cpp::meta::arithmetic Arithmetic>
struct caster<Arithmetic>
{
    static Arithmetic operator()(const value& v)
    {
        if (v.is<number>())
        {
            return v.as<number>().as<Arithmetic>();
        }
        else if (v.is<boolean>())
        {
            return v.as<boolean>() ? Arithmetic(1) : Arithmetic(0);
        }
        else if (v.is<string>())
        {
            std::string_view ctx = v.as<string>();
            auto result = type_caster<Arithmetic, std::string_view, cpp::error_policy::optional>()(ctx);

            if (result)
            {
                return *result;
            }
            else
            {
                throw std::runtime_error("Failed to convert string to number");
            }
        }
        else
        {
           throw std::runtime_error("Value is not a number");
        }
    }
};

template <std::ranges::range Container>
struct caster<Container>
{
    static Container operator()(const value& v)
    {
        using ValueType = typename Container::value_type;

        if constexpr (cpp::meta::pair_like<ValueType>)
        {
            // object
            using KeyType = std::tuple_element_t<0, ValueType>;
            using MappedType = std::tuple_element_t<1, ValueType>;

            if (v.is<object>())
            {
                return v.as<object>() | std::views::transform([](const auto& pair) {
                    return std::pair<KeyType, MappedType>(pair.first, caster<MappedType>()(pair.second));
                }) | std::ranges::to<Container>();
            }
            else
            {
                throw std::runtime_error("Value is not an object");
            }
        }
        else
        {
            // array
            if (v.is<array>())
            {
                return v.as<array>() | std::views::transform(caster<ValueType>()) | std::ranges::to<Container>();
            }
            else
            {
                throw std::runtime_error("Value is not an array");
            }
        }
    }
};

}  // namespace detail

inline std::string dumps(const value& x)
{
    return detail::caster<std::string>()(x);
}

inline void dump(const value& x, const char* filename) 
{
    auto context = dumps(x);
    write_file(context, filename);
}

} // namespace cpp::config::json

// template <typename CharT>
template <>
struct std::formatter<cpp::json::value, char> 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const cpp::json::value& value, FmtContext& ctx) const
    {
        auto result = cpp::json::dumps(value);
        return std::ranges::copy(result, ctx.out()).out;
    }   
};

template <typename Target>
struct cpp::type_caster<Target, cpp::json::value, cpp::error_policy::exception>
{
    static auto operator()(const cpp::json::value& v)
    {
        return cpp::json::detail::caster<Target>()(v);
    }
};
